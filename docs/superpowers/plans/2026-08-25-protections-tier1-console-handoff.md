# Protections Tier 1 — console verification handoff

Branch `feat/protections`, ready to merge into `main`. Build tag at merge time:
`prot-final-1` (`src/platform/build_tag.h`).

**Nothing in this document has been run on hardware.** No `.prx` was deployed,
no kernel log was read, no game was launched during this subsystem's
development — there was no human at the console. Every "expect X" statement
below is a prediction derived from static analysis (disassembly, the leaked
`dev_ng` source, and the PC build), not an observation. Treat every checklist
item as a first-time test, not a regression check.

This document exists because the per-task work that produced it lived in
`.superpowers/sdd/2026-08-25-protections-tier1/` (gitignored, deleted after
merge). If you need the blow-by-blow derivation for any anchor, it is in
`E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md` — see §8 below.

---

## 1. State of play

Twelve filters, one table (`src/protections/registry.cpp`), ground truth for
all of this — re-check it if this document and the binary ever disagree.

| filter | id | wired (has a detour) | can block (Enforce does something) | stays Log-only forever | why |
|---|---|---|---|---|---|
| Self Test | 0 | no | no | yes — no detour to promote | proves the report path only |
| Skeleton Extension | 10 | **yes** — `0x1E27300` | yes | no — Enforce-eligible | sole caller already null-tests the return |
| Fragment Physics | 11 | no | no | yes | PS4 twin of `fragment_physics_crash_2` never found |
| Invalid Decal | 12 | **yes** — `0x6E13F0` | yes | no — Enforce-eligible | retail's own null check skips the body on that branch anyway |
| Searchlight | 13 | **yes** — `0x108F770` | yes | no — Enforce-eligible | a report means the call would have faulted |
| Task Ambient Clips | 14 | **yes** — `0xD08A90` | yes (`can_block=true`) | **yes — by judgement, not by table** | fires in ordinary play; see §4 |
| Task Parachute | 15 | **yes** — `0xE40A20` | yes | no — Enforce-eligible | returns `FSM_Continue`, the original's own value on that path |
| Render Ped | 16 | **yes** — `0x7FCA00` | yes | no — Enforce-eligible | bails 13 slots early, costs one entity for one frame |
| Render Entity | 17 | **yes** — `0x7E0CA0` | yes | no — Enforce-eligible, but see §6.1 | writes the sentinel the caller expects for "no entry produced" |
| Render Big Ped | 18 | **yes** — `0x7FC4A0` | yes | no — Enforce-eligible, **flip last**, see §6.2 | same shape, sentinel not matched store-for-store |
| Pool Exhaustion | 19 | no — identified but unhookable | no | yes | `0x1EF6A00` prologue's forced 15-byte steal contains a `jz rel8` GoldHEN copies unrelocated |
| Reliable Allocator | 20 | **yes** — `0x1A541E0` | no (`can_block=false`) | yes, until a recovery is written | retail null-checks every `AllocCritical` return — nothing to refuse |

9 of 12 are wired. 7 are Enforce-eligible today (Skeleton Extension, Invalid
Decal, Searchlight, Task Parachute, Render Ped, Render Entity, Render Big
Ped). 5 are not, for five different reasons — four of them (`self_test`,
`fragment_physics`, `pool_exhaustion`, `reliable_alloc`) are enforced
*structurally*: `can_block = false` makes the menu offer only `Off`/`Log` and
stops the drain from ever printing `BLOCK` for something that blocked
nothing. `task_ambient_clips` is the one row that *can* block and must simply
not be asked to — see §4.

All twelve default to `Log` (`default_mode = mode::log` in every row), and
every mode persists to `config.json` via `add_savable`.

---

## 2. Deploy and watch

```bash
python -c "
import ftplib, io
data = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build-wsl\InsulinGTAV.prx','rb').read()
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
f.storbinary('STOR /data/GoldHEN/plugins/InsulinGTAV.prx', io.BytesIO(data)); f.quit()
print('deployed', len(data), 'bytes')"
```

Kernel log, started **before** launching the game so the boot line is caught:

```
nc 10.10.10.236 3232
```

Plugin lines are prefixed `IGV`.

**The build tag is the only proof of what is running.** `FTP RETR` is
unreliable on GoldHEN — it returns a fixed-size blob regardless of the actual
file, so downloading the `.prx` back and eyeballing its size or a hex dump
proves nothing. The only trustworthy check is the boot line:

```
BUILD=<tag> module_start base=0x...
```

At merge time the tag is `prot-final-1`. If the tag on console does not match
the tag in `src/platform/build_tag.h` on the commit you are testing, stop —
every step below is meaningless against a stale build. `INSULIN_BUILD_TAG` is
also shown in the in-game debug panel; the two must agree by construction
since both read the same macro.

---

## 3. The install-time hazard — read this before anything crashes

All nine wired detours install at once, from a single one-shot call
(`protections::install_enabled_filters()`) inserted into `menu::tick` just
above the `player_valid()` gate that also drives native-hash resolution
(`src/menu/menu.cpp`, near line 409 as of the final commit). This runs the
first frame the player is valid, whether or not the Protections menu has ever
been opened — every filter defaults to `Log`, so by design they install
themselves without the user doing anything.

**Two of the nine targets (`render_ped`, `render_entity`) run hundreds of
times per frame.** This is the first time this project has patched a *hot,
actively executing* function — every earlier hook (`game_thread.cpp`,
`heap_guard.cpp`) installs from plugin init, before the game is running at
all.

Here is the mechanism, read out of GoldHEN's own `Detour.c`:

1. `Detour_DetourFunction` first `memset`s the target's prologue to `0x90`
   (NOP), for the full stolen-instruction length.
2. It then calls `sceKernelMprotect` — a **syscall** — to make the trampoline
   page executable.
3. Only after that syscall returns does it write the 14-byte absolute jump
   into the now-NOPed prologue.

For the duration of step 2 — a real syscall, not a few cycles — the target
function's prologue is nothing but NOPs. If any thread calls into that
function during that window, it executes the NOPs, falls off the end of the
stolen bytes, and resumes execution mid-function with none of the expected
`push`es having happened. On `render_ped` or `render_entity`, hit hundreds of
times a frame, that window is not a theoretical race.

**If the game dies the instant protections come online — not during play,
not on a specific action, but right at the point all nine detours land —
this install race is the first suspect, not the filter logic in
`hooks_*.cpp`.** A crash a few seconds or minutes into normal play, after the
`prot detour installed` lines have all appeared, is a different kind of bug
and points at a filter, not at this race.

Two mitigations were considered and neither was applied, because neither
could be tested without a console:

- **Move the install earlier, into `module_start` after `menu::build()`.**
  Installing before the game thread is running removes the concurrent-entry
  risk, matching how `game_thread.cpp` and `heap_guard.cpp` already do it.
  Rejected only because it was untestable at the time — nothing here rules
  it out, and it is the more promising direction if this race turns out to
  be real.
- **Keep the current late install point and accept the window.** This is
  what shipped. The install point was originally chosen to avoid detours
  landing while the game is still loading (a different, earlier-discovered
  hazard — see the `progress.md` DEFECT-2 ruling), and moving it later
  turned out to trade one race for a worse one. Nobody could test either
  placement, so the current one ships documented rather than silently
  "fixed" on guesswork.
- **Staggering the nine installs one per frame** was also considered, to at
  least make a crash attributable to a specific detour. Rejected: that is
  new, untested logic sitting directly in the boot path right before a
  console session — exactly what this project's own boot rules warn
  against.

If you do hit an install-time crash, the klog crash dump will give an `RVA`
(`RIP - base`); check it against §8 before assuming it is one of the render
guards — it might not be.

---

## 4. Verification sequence, in order

Do these in order. Each stage assumes the previous one passed.

### 4.1 Framework self-test (Task 4) — no reverse engineering required

This exercises the whole pipe — ring → coalescer → drain → klog →
rate-limited notification — using the one filter that has no detour and
therefore cannot crash anything.

1. Deploy, start the kernel log, launch. Confirm `BUILD=prot-final-1
   module_start base=0x...`.
2. Open the menu → Protections. Expect twelve dropdown rows, a `Diagnostics`
   break with **Fire Self Test** and **Report Counters** buttons, then a
   `Local Self-Care` break with three unrelated toggles (**Anti Fire**,
   **Anti Ragdoll**, **Explosion Proof** — these are not network filters,
   see §7).
3. Press **Fire Self Test**.
   - Expect an on-screen notification: `Detected Self Test from player 3`.
   - Expect a klog line: `prot would-block Self Test player=3 a=abcdef01 b=00000007`.
4. **Self Test cannot reach Enforce, and that is correct — do not try to
   force it.** It has no detour, so it blocks nothing; its dropdown offers
   only `Off`/`Log`. If you see an Enforce option here, the build is not
   `prot-final-1` — an earlier build had this bug (`can_block` had not yet
   been added) and it was fixed specifically because "Self Test → Enforce →
   Fire" used to write `prot BLOCK Self Test` into the kernel log while
   blocking nothing, the exact kind of lie this subsystem exists to
   prevent.
5. Press Fire Self Test rapidly ten times. Expect ten klog lines but roughly
   one notification per second — the per-filter rate limit.
6. Press **Report Counters**; the total must match how many times you fired.
7. Set Self Test to `Off`, fire again. The button calls `report()` directly
   regardless of mode, so the record still drains — you are checking that
   the line reads `would-block` (never `BLOCK`), not that nothing is
   logged.
8. Quit and relaunch. Every Protections row (filters and the three
   Local Self-Care toggles) must come back with the mode/state you left it
   on — `add_savable` round-tripping through `config.json`.

If any of this fails, stop — every guard below drains through the same ring
and the same coalescer, so a framework bug here invalidates everything after
it.

### 4.2 Each guard, in Log — read §5 before judging any report

Deploy with every filter at its default (`Log`). Play normally. For each
guard, "a report" does **not** automatically mean "the anchor is wrong" —
read the per-filter semantics in §5 before concluding anything. In short: two
guards (`task_ambient_clips`, `task_parachute` under one specific value) are
*expected* to report in ordinary play; the rest should stay silent, and a
report from those is worth capturing verbatim (the full `a=... b=...` line)
before deciding what it means.

Confirm the install line for each filter as you enable it:
```
prot detour installed @ base+0x<rva>
```
A missing line for an enabled filter means `install_detour` refused it —
check `g_eboot_base` is resolved, and check the klog for
`protections: detour @rva 0x... FAILED` or `... rva is zero (unfilled
anchor)`.

Suggested order (matches how the guards were built and reviewed, not a
requirement): Skeleton Extension → Invalid Decal → Searchlight → Task
Parachute → Task Ambient Clips (Log only, see §4.3) → Render Ped → Render
Entity → Render Big Ped → Reliable Allocator.

### 4.3 Enforce promotions

Only after a clean pass in Log. Flip **one filter at a time**, not all seven
at once, so a regression is attributable.

- **Do not flip Task Ambient Clips to Enforce in this pass, or possibly
  ever**, without the dedicated experiment in §5. It has `can_block = true`
  and a real detour — nothing in the code stops you — but doing so is
  expected to freeze ped idle animations. See §5.
- **Flip Render Entity only after §6.1 is resolved.**
- **Flip Render Big Ped last, and only after §6.2 is resolved.**
- The other five (Skeleton Extension, Invalid Decal, Searchlight, Task
  Parachute, Render Ped) have no known open concern and are safe to promote
  once their Log pass is clean.
- Reliable Allocator and the three `can_block = false` filters
  (Self Test, Fragment Physics, Pool Exhaustion) do not offer Enforce in the
  menu at all — this is enforced by the UI, not just documentation.

For every promotion: repeat the activity that exercises the guard (see §5
per filter) and confirm the legitimate path still works end to end — a guard
that silently breaks a legal action is worse than the crash it prevents.

---

## 5. What a report actually means, per guard

The kernel log line shape is always:
```
prot <would-block|BLOCK> <Filter Name> [player=<n>] a=<hex> b=<hex>[ +N more]
```
`BLOCK` only ever appears for a filter with `can_block = true` while it is
in `Enforce`; everything else — including a filter in `Enforce` that cannot
block anything — prints `would-block`. `+N more` (added by the coalescer, see
§3 of `progress.md`) means N further occurrences of the same filter arrived
before this record drained; the payload shown is the *first* occurrence, not
the latest.

**Task Ambient Clips (`0xD08A90`) WILL fire in normal solo and online play.
This is expected, not a bug.** The game itself treats a null "conditional
anims group" pointer as legal — `CTaskAmbientClips::Start_OnUpdate`
null-checks the exact field the guard tests before using it. This is
YimMenu's own known imprecision (its source comment says the guard "doesn't
block the crash completely"), not evidence of a wrong offset; the `+0x100`
field offset was independently verified unshifted between the PC and PS4
builds across six neighbouring fields specifically to rule that out. A
steady trickle proportional to ped count is the expected shape; a burst tied
to one specific remote player is what the filter exists to catch. **Do not
back out this anchor because it reports in solo play, and do not promote it
to Enforce without a deliberate test that specifically watches ped idle
animations for freezing** — returning 0 from `UpdateFSM` on this state skips
`TaskSetState` entirely, and the ped would be pinned in `State_Start` with no
ambient clip ever chosen, silently, with no assert to catch it (`taskAssertf`
is compiled out in `__FINAL`).

**Task Parachute (`0xE40A20`): the `a=` value tells you which of two very
different situations you're looking at.** The guard fires when any of three
pointer links is null, but retail itself null-checks two of the three — only
the first is genuinely unchecked in retail.
- **`a=00000000`** — the first link (`*(this+0x10)`, `GetObject()`) was
  null. Retail *never* checks this one. **This is the fatal case the guard
  exists to catch** — a report here means the guard is doing its job, not
  that the anchor is wrong.
- **`a=00000001`** — the first link was fine; the null was further down a
  chain retail already null-checks (`if (v7) { if (*(v7+64)) { ... } }`).
  This is a **legal transient**, most likely the anim director not yet
  attached while the parachute streams in. It is retail's own normal
  behavior on that tick and is **not** grounds to back the anchor out, even
  though it will show up around ordinary parachute deploys.

Only a flood of `a=00000000` in solo play would genuinely indict the anchor.

**Invalid Decal (`0x6E13F0`) now observes all 12 palette components, not
just component 2 as the original brief specified** — the fix wave widened
the guard because retail's own null check (`if (v29)`) skips the whole body
for *every* component on that branch, so widening loses nothing and closes a
gap where the original key covered roughly 1/12 of the real crash surface.
**Read "zero reports in Log" against that wider surface** — the completion
criterion is stronger now than when the plan was written, not weaker.
`detail_a` = component index (0–11), `detail_b` = which link was bad (1 =
ped pointer null; 2 = draw handler null, benign/legal; 3 = palette-set null
on the head-blend + hair branch — can be an occasional benign false
positive since the hook doesn't evaluate the branch selector the game uses;
a flood of `3`s means the guard's assumption about which branch is running
needs revisiting, not that the offsets are wrong).

**Searchlight (`0x108F770`): unlike every other guard so far, there is no
benign case.** Both dereferences it guards are unconditionally fatal in
retail — expect **zero reports, in both Log and Enforce, in ordinary play**.
A single report is a real would-be crash worth capturing. `detail_a`: 1 =
ped null, 2 = no vehicle, 3 = vehicle has no weapon manager, 4 = weapon
count outside `[1,6]` (`detail_b` carries the count), 5 = no `CSearchLight`
found among the vehicle's weapons (a steady stream of `5` specifically would
point at the `CSearchLight` vtable constant `0x306FD10` being wrong — nothing
else produces that pattern).

**Skeleton Extension (`0x1E27300`): retail has no bound check here at
all** (`TrapGE` compiles to nothing under `__FINAL`), which makes this the
cleanest Log semantics of any guard on the branch — **a report is never a
legal transient.** It means `m_Count` was already at or past capacity (32)
when the game asked for a slot: the array is one call away from an
out-of-bounds write, or, if the entity already carries an extension, a pure
lookup that happens to also be a state that should never occur. Either way,
reaching 32 at all is abnormal. `detail_a` is the raw count read (a value
above 32 tells you how far past capacity writes have already landed — each
overflowing element stomps 80 bytes at `base + 80n .. base + 80n + 0x4F`,
see anchors §11 for the exact victims at n=32..35). This guard only exercises
at all in first-person view with attachments (parachute, weapons, props) —
if it never fires, first confirm you actually triggered the code path before
reading that as a clean pass.

**Reliable Allocator (`0x1A541E0`): detection only, and every report is a
post-mortem, not a warning.** Retail null-checks every single
`AllocCritical` call site and has *already* fired its own fatal
out-of-memory callback for that connection by the time the hook's return
value goes null — reclaim was attempted, retried, and failed before you see
the log line. There is nothing left to refuse: the hook never calls
`should_block()`, `Enforce` is not offered in the menu, and a report means
"the network heap was genuinely dry and that endpoint's session is likely
already tearing down," not "a crash was narrowly averted." Expect zero
reports in a healthy 20-minute session; a report is worth capturing (`a` =
requested size, `b` = that connection's cumulative failed-allocation count)
and raising as a decision about whether to write a recovery — not a sign
anything here is broken.

---

## 6. Three things to resolve before specific Enforce flips

From the final whole-branch review's fix wave (`final-fix-report.md`), three
Enforce-gated concerns were found and explicitly left open rather than
guessed at:

### 6.1 `render_entity` — bit-31 divergence in the block path

On the block (`!v8`) path, the real PS4 function computes
`bits 30–31 of +4 = the low TWO bits of a4`, via
`... | (a4 << 30)` after separately clearing bit 31. The guard's block path
writes `(a4 & 1) << 30` — only bit 30, leaving bit 31 always clear. For any
`a4` whose bit 1 is set, the guard's synthesized "no entry" sentinel differs
from what the original would have written by exactly that one bit. This is
Enforce-only (Log mode chains straight through to the real function and is
unaffected) and does not touch the separate `bool`→`unsigned char` fix
already applied to keep Log-mode byte-for-byte inert. **Resolve — decide
whether `(a4 << 30)` (masking to the low two bits, matching the original) is
safe to ship, or find out empirically whether bit 31 being wrong here is
ever observable — before flipping Render Entity to Enforce.**

### 6.2 `render_big_ped` — sentinel not matched store-for-store

The block path writes only `*(out+4) = -2` and returns `out + 0x14`, leaving
`+0`, `+2`, `+12`, `+16`, `+17` of the output record holding whatever the
caller's buffer already contained. This matches what the real PS4 function
and its PC twin both do — it is not a deviation — but it means the
caller-honors-just-the-sentinel assumption has never been exercised outside
of static reading, because the path only executes when the draw list is
already at capacity, which is exactly the condition Log mode exists to
confirm never happens in normal play. **Flip this one last of the seven
Enforce-eligible guards**, and watch specifically for garbage read from an
uninitialized big-ped draw record if it ever does trip in Enforce.

### 6.3 `task_ambient_clips` — Log-only restraint is a judgement call, not a structural guarantee

Covered in full in §5. Restated here because it belongs on this list: unlike
the other three non-blocking filters, this one has `can_block = true`
because the hook genuinely can refuse. Nothing in code prevents someone from
setting it to Enforce; the only thing stopping that is this document and the
operator reading it. If that restraint should become structural, the
cheapest fix is a fifth `can_block = false` row — at the cost of overloading
what that field means (three other rows use it because there is *no detour
or nothing to block*; this one would use it to mean *there is something to
block and we choose not to*).

---

## 7. What's deliberately not here

Three things a reader might go looking for and not find, so nobody wastes
time re-deriving what was already ruled out:

- **`fragment_physics_crash_2` was never identified on PS4.** The PC
  function (`0x7FF6FF5E80BC` / PC RVA `0x15080BC`) is fully characterized —
  an oriented-bounding-box overlap test, no string anchor, no `[ps4-bridge]`
  entry, and its floating-point constant pool (`1e-6`, `0.003`) is too
  generic to fingerprint (40+ and 80+ hits respectively across `.rodata`).
  See anchors §10 for what was tried and what would crack it next
  (structural diff of the physics module is the best next lead). The
  registry keeps `install = nullptr`; nothing hooks this filter.
- **`pool_exhaustion` was identified with certainty but cannot be safely
  hooked with the current installer.** `rage::fwBasePool::New` @
  `0x1EF6A00` is confirmed via three independent routes. Its entry block is
  only 12 bytes, forcing a 15-byte prologue steal (GoldHEN needs to end on
  an instruction boundary at or past 14 bytes), and that 15-byte range
  contains a `jz rel8` branch at offset 10 whose target GoldHEN's stub does
  **not** relocate when it copies the stolen bytes — the copied branch would
  jump to the wrong place, which happens to be exactly the pool-empty path
  this guard exists to observe. See anchors §12 for the full route-ranking
  analysis of how a future task could unblock this (a relocating trampoline
  in `detour.cpp`, ranked above `DetourMode_x32`, which independently has
  three of its own failure modes documented there).
- **`reliable_alloc`'s recovery is unwritten, on purpose, pending
  evidence** — the plan explicitly stages this filter as detection-only
  until the detection has actually been seen firing on a real session.
  Groundwork for a future recovery is recorded, not built:
  `rage::netConnection::ReclaimAllMem` @ `0x1A53E50` **is already the queue
  walk** YimMenu hand-rolls on PC (reclaim, drain the resend queue freeing
  unacked reliables, drain the send queue, dispose the inbound out-of-order
  tree) — so a future recovery is a **call to an existing engine function**,
  not a reimplementation. It has no direct callers in the current build,
  which is why nothing already invokes it. See anchors §13 for the full
  field map.
- **The "Local Self-Care" toggles** (Anti Fire, Anti Ragdoll, Explosion
  Proof) visible below the filter list in the Protections menu are **not**
  network filters and are unrelated to everything above — they're pre-existing
  local state toggles kept in place (Anti Fire has no home anywhere else in
  the menu) and labelled separately on purpose. Don't read a report or
  absence of one against them; they don't go through the ring at all.

---

## 8. Where the evidence lives

`E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`, sections 1–13.
Every RVA in the table in §1 has a corresponding section there with the
decompiled listing that justifies it, cross-checked against the PC build
and/or the leaked `dev_ng` C++ source where available, plus a prologue-safety
measurement (instruction boundaries, whether the stolen bytes contain a
RIP-relative operand or a relative branch) for every hooked target.

Section map:
- §1–5: draw-handler manager global, count offset/capacity, `render_entity`,
  `render_big_ped`, `render_ped`.
- §6–7: `task_parachute`, `task_ambient_clips` (includes the FSM
  OnEnter/OnUpdate/OnExit encoding, read from the disassembly rather than
  assumed).
- §8–10: `invalid_decal`, `searchlight`, `fragment_physics_crash_2` (unfound
  — full list of what was tried).
- §11–12: `skeleton_extension`, `pool_exhaustion` (unhookable — full
  route-ranking for unblocking it).
- §13: `reliable_alloc`, plus the recorded groundwork for a future recovery.

**One known erratum in that document, parked rather than fixed:** line
~1719 (the "Group 4" preamble before §13) still reads *"Where the leaked
source and the binary disagree (they do, in one place — see 'Version drift'
below), the binary wins and the drift is written down."* That forward
reference is stale. The "version drift" paragraph it points to, inside §13,
was corrected during the fix wave to say the opposite — **there is no
drift**: `netConnection::QueueOutOfMemory` in the leaked source already
calls its callback synchronously (the source comment says so explicitly),
matching the binary exactly; the earlier report had conflated it with a
different function, `netConnectionManager::QueueOutOfMemory`, which really
does queue. So §13 no longer contains the disagreement the preamble
promises. This is purely a stale cross-reference — nothing about the
`reliable_alloc` anchor itself is in doubt — and is first on the list for
whoever next touches that document.
