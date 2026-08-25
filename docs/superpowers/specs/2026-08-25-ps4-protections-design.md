# Protections: filter hostile network traffic the way YimMenu does, on a network thread that cannot call natives

YimMenu V1 protects a GTA V PC client by detouring the network receive path and
dropping hostile traffic before the game acts on it. This ports that idea to the
PS4 build. The filters are largely the same logic; almost everything else is
different, because on PS4 the code runs on the network thread, where the menu's
usual tools — natives, `stl::` allocation, notifications — are all unavailable.

The port targets **CUSA00411 v1.57** only. Every address is an RVA against ELF
base 0, resolved against `rage::invoker::g_eboot_base` at install time.

## Scope

Sessions are **LSO private-server** sessions. That decides what is worth
porting: the threat is another modder in your session, not Rockstar matchmaking.
YimMenu's session and ROS layer — join-request filtering, presence attribute
spoofing, RID-join blocking — protects against attacks that route through R*
services, and those services are not in the path here. Those hooks are **out of
scope**, recorded in "Deliberately not ported" below so the decision is not
silently re-litigated later.

That leaves **23 of YimMenu's 26** protection hooks, two of which are ported in
part rather than whole. The matchmaking and session-advertisement hooks live
outside `hooks/protections/` and are not counted here; none of them are in
scope either, for the same reason.

## What exists today

`menu/base/submenus/protections.cpp` is a placeholder. Its own comment is
accurate and states the problem this spec solves:

> Ozark's protections work by hooking the network-event receive path and dropping
> hostile script events before the game acts on it. This port has no such hook,
> so none of that is available here yet.

It offers three local self-care toggles (anti-fire, anti-ragdoll, explosion
proof) that keep a state from sticking. Those are not protections in the
YimMenu sense and this design does not build on them; they move to `Player >
Proofs`, where they belong, and the Protections submenu is rebuilt.

What *is* available and verified:

| Need | Where | Note |
|---|---|---|
| function detour + chain | `rage/heap_guard.cpp` | `Detour_Construct` / `Detour_DetourFunction` / `Detour_Stub`; working reference |
| eboot base | `rage::invoker::g_eboot_base` | all RVAs made absolute against this |
| per-frame callback on the script thread | `game/game_thread.h` `set_frame_callback` | where reporting is drained |
| queue work to the script thread | `game/game_thread.h` `run_on_game_thread` | takes `void(*)()` — **no payload argument** |
| notifications | `menu/base/util/notify.h` `stacked` | script thread only |
| kernel log, survives a crash | `platform::klogf` | prefixed `IGV` |
| config persistence | `util/config.h` | `add_savable` on menu options |
| player list | `game/player_list.h` | 32 slots |

## The three constraints that shape everything

**1. Hooks run on the network thread.** This is the single most important
difference from YimMenu, and it invalidates a direct code port. YimMenu's
protection hooks call natives (`NETWORK_IS_ACTIVITY_SESSION`), allocate
(`std::string target = "<UNKNOWN>"`), format (`std::vformat`) and push
notifications, all from inside the hook. Doing any of that here breaks boot-rule
#1 — natives only run on the game's script thread — and the game closes itself.

So: **a filter is a pure function of memory it can read.** No natives, no
allocation, no notification, no logging that formats. A filter that wants to
report something writes a fixed-size record into a ring buffer and returns.

**2. `run_on_game_thread` carries no payload.** It takes a bare function
pointer. Reporting therefore cannot be "queue a closure describing what
happened"; it has to be "write a record, and let a fixed drain function on the
other side read it". That is what forces the ring-buffer design rather than a
work queue.

**3. Enforcement is a separate decision from detection.** Every filter has a
mode — `Off`, `Log`, `Enforce` — persisted per filter. New filters ship in
`Log`: they run the full detection, write the report record, and then call the
original anyway. Enforcement is flipped per filter, deliberately, after the log
shows the detection firing only when it should. This is the verification
strategy made structural instead of a habit, and it is the reason the design
tolerates porting filter logic whose PS4 field offsets are not yet confirmed —
a wrong offset in `Log` mode is a bad log line, not a crash and not a dropped
legitimate packet.

## Approach: per-object detours, not a central bit-buffer parser

YimMenu's `received_event` hook intercepts events *before* deserialisation and
re-parses the raw `datBitBuffer` itself, per event id. The attraction is that it
catches attacks aimed at the deserialiser. The cost is that it depends on the
exact wire layout — field order and bit widths — of every event, and those drift
between title updates. Porting them on faith to a 1.57 build risks misparsing,
and a misparse means silently dropping legitimate events: a failure that is
quiet, intermittent, and very hard to attribute.

This design hooks the **deserialised event objects** instead. `Decide` receives
a fully-constructed C++ event and the sending `CNetGamePlayer*`; the filter reads
typed fields at struct offsets, which can be verified live over Frame4, rather
than re-deriving a bit layout that cannot.

The trade is real and worth stating: this does not catch an attack that crashes
the deserialiser itself, because by the time `Decide` runs, `Handle` has already
completed. It is not foreclosed — the tables that a central pre-deserialise hook
would need are located (below), and if `Log` mode ever shows traffic that never
reaches `Decide`, that hook is the follow-up. Starting there would mean building
the riskiest, least verifiable piece first.

## Verified anchors

Confirmed by decompilation this session:

| Anchor | RVA | Evidence |
|---|---|---|
| `CScriptedGameEvent` vtable | `0x30A8E80` | stamped as first member write in `sub_16C5800` |
| `CScriptedGameEvent::Handle` | `0x16C5E50` | vtable slot 7; reads u32 → `this+0x224`, then `8*count` bytes → `this+0x70` |
| `CScriptedGameEvent::Decide` | `0x16C5F10` | vtable slot 8; `(this, CNetGamePlayer* from)`; guards `count != 0 && count <= 0x1B0` |
| script event args | `this+0x70` | array of 32-bit args |
| script event arg count | `this+0x224` | count, not bytes; max `0x1B0` (432) |
| `netEventMgr::Update` | `0x1E79C10` | called from `NetworkEventUpdate` under the `"Event Manager"` label |
| `netEventMgr::Init` | `0x1E79890` | clears both tables, 91 slots |
| `netGameEventMgr::RegisterEventFactory` | `0x1E7A6C0` | `(mgr, id ≤ 0x5A, fn, name)` |
| event factory table | `mgr + 242648` | 91 × 8, from Init and Register |
| event name table | `mgr + 243384` | 91 × 8; PC is `243376` — 8 bytes apart |
| `netEventMgr::HandlePackedEventReliablesMsg` | `0x1E7A470` | ack/reply send side |
| `g_NetworkObjectMgr` | `0x385F5E0` | `Update` via vtable+32 under the `"Object Manager"` label |
| `g_NetworkArrayMgr` | `0x385F5F0` | `"Array Manager"`, vtable+80 |
| `g_NetworkSession` | `0x385F5F8` | same function |

The name-table offsets being 8 bytes apart between PC and PS4 is the strongest
single signal in this document: the event manager layout is near-identical
across the two builds, so YimMenu's structural assumptions about it hold here.

From prior work (`analysis/PC_SIG_PORT.md`, `NETCODE_PROTOCOL.md`), already
resolved and directly needed:

| Anchor | RVA |
|---|---|
| `g_SyncTree_Automobile … Train` (13 singletons) | `0x39B12B8` … `0x39B4E80` |
| `netSyncTree::ApplyNodes` | `0x1E9D500` |
| `CNetworkObjectMgr::GetNetworkObject` | `0x1EE5C80` |
| `netObject::GetSyncTreeForCloneType` | `0x1615E10` |
| `g_WeaponInfoArray` | `0x385E850` |
| `g_sysMemAllocator` | `0x311BD80` |
| `CGiveControlEvent` vtable | `0x30A7D40` |
| `g_NetworkPlayerMgr` | `0x3080BF0` |
| `CNetworkPlayerMgr::GetPlayerFromIndex` | `0x1514480` |
| `CNetworkPlayerMgr::GetLocalPlayer` | `0x1613060` |
| `g_CPedFactory` | `0x3278708` |

Struct offsets, live-verified in a 2-player private-server session:

```
CNetGamePlayer +0x31 (u8)  physical player index      [the per-player key]
CNetGamePlayer +0xB0       CPlayerInfo*
CPlayerInfo    +0x280      CPed*
CPed           +0xD0       netObject*
netObject      +0x08 (u16) object type   (11 = PLAYER)
netObject      +0x0A (u16) object id     (13-bit)
netObject      +0x18       sync data — 0 on remote clones; the ownership tell
netObject      +0x50       m_pGameObject → CEntity
```

## Components

```
src/protections/
  registry.{h,cpp}     filter table; per-filter mode; install/uninstall
  report.{h,cpp}       lock-free SPSC ring; drain on the script thread
  players.{h,cpp}      per-player block bitmasks, keyed on CNetGamePlayer+0x31
  hooks_guards.cpp     Tier 1 — local crash guards
  hooks_events.cpp     Tier 2 — script events + per-event filters
  hooks_clone.cpp      Tier 3 — clone create/sync/remove, array update, messages
  hooks_syncnode.cpp   Tier 4 — sync-node validator
menu/base/submenus/
  protections.cpp      rebuilt: one option per filter, tri-state mode
  protections_log.cpp  new: recent blocks, readable in-game
  network_players.cpp  extended: per-player block toggles
```

### registry

One table entry per filter:

```cpp
enum class mode : uint8_t { off, log, enforce };

struct filter {
    const char* name;        // stable id, also the config key
    uint8_t     tier;
    mode        current;     // read from hook threads, written from the menu
    bool        installed;
    bool      (*install)();  // detour install; idempotent
};
```

`current` is a plain `uint8_t` read without a lock. A torn read is impossible for
a byte, and the worst case of a race is one packet handled under the previous
mode — acceptable, and much cheaper than synchronising a network hotpath.

Detours install once, at first enable, and are never removed: uninstalling a
detour while another thread is inside the trampoline is a crash the design
should not risk. `Off` means the hook returns to the original immediately, which
costs a predictable-branch and nothing else.

### report

A single-producer-single-consumer ring of fixed-size records. Producers are hook
threads; the consumer is the frame callback.

```cpp
struct record {
    uint16_t filter_id;
    uint8_t  player_index;   // 0xFF when unknown
    uint8_t  flags;
    uint32_t detail_a;       // filter-defined; event hash, object id, node id
    uint32_t detail_b;
};
```

No pointers and no strings: a record must stay valid after the producing thread
has moved on, and the consumer must be able to render it without touching game
memory that may since have been freed. Filters are numbered, and the names live
in the registry on the consumer side.

Multiple producer threads are possible in principle. The ring uses a single
atomic tail with a compare-exchange claim, which makes it MPSC without a lock.
Overflow drops the newest record and increments a counter — reporting must never
block a network thread, and a burst of identical blocks is not information worth
stalling for.

The frame callback drains the ring, writes each record to the kernel log via
`platform::klogf` (never `LOG_CUSTOM`, which needs FTP afterwards and is useless
when chasing a crash), and pushes a rate-limited notification. Rate limiting
matters: a sound-spam attack produces hundreds of blocks per second, and a
notification per block would be its own denial of service.

### players

A per-player block state, indexed by physical player index 0..31:

```cpp
struct player_blocks {
    uint32_t net_events;     // bitmask over 32 players
    uint32_t clone_sync;
    uint32_t clone_create;
};
```

Three `uint32_t`s, read directly in hooks with a bit test. Written from the menu
thread. This is the whole per-player mechanism — no player database, no
infraction history, no reactions engine. Those are a second subsystem, and this
spec does not build them.

The Players submenu gains three toggles per player, bound to the bit for that
player's index.

## Tier 1 — local crash guards

Standalone detours with a null or bounds check. No session required, so they are
testable the moment they are written; they also protect single-player. Each is a
handful of lines and depends on no shared infrastructure beyond the registry.

| Filter | Guard |
|---|---|
| `skeleton_extension` | refuse when the extension count is already ≥ 32 |
| `fragment_physics` | reject null float args |
| `invalid_decal` | reject when the `+0x48 → +0x30 → +0x2C8` chain ends in null |
| `searchlight` | reject when the ped has no searchlight |
| `task_ambient_clips` | require `this+0x100` non-null |
| `task_parachute_object` | require the `+0x10 → +0x50 → +0x40` chain for the `(1,1)` case |
| `render_ped` | bail when the draw-handler count ≥ 499 |
| `render_entity` | bail when ≥ 512, writing the sentinel the game expects |
| `render_big_ped` | bail when ≥ 512, writing the sentinel |
| `pool_exhaustion` | log-only; report the caller RVA when a pool allocation fails |
| `reliable_alloc` | on allocator exhaustion, free queued messages and retry |

**None of the eleven target functions have been located on PS4 yet** — finding
them is the bulk of this tier's work and each is a named RE task in the plan.
They are small, string- or structure-anchored, and independent of each other,
which makes them good parallel work. `render_*` additionally needs the
draw-handler-manager global and the `+0x14730` count offset re-derived — the PC
constants are not portable.

`reliable_alloc` is the one Tier 1 filter that is not a guard but a recovery
path, and it is the most intricate: it walks two message queues, removes
unacked reliables, and frees. It depends on `g_sysMemAllocator` (`0x311BD80`)
and on the connection and queue layouts, none of which are mapped yet. It is
sequenced last in the tier for that reason.

## Tier 2 — event filters

The highest-value tier, and the one whose anchors are already confirmed.

**Script events.** A detour on `CScriptedGameEvent::Decide` (`0x16C5F10`) sees
every incoming script event with its sender. `args[0]` is the event type; the
filter switches on it and returns without chaining to drop the event.

This single hook covers the whole family YimMenu protects: bounty, CEO money,
clear-wanted-level, force-mission, forced teleport, MC teleport, personal-vehicle
destroyed, remote off-radar, rotate-cam, send-to-cutscene, send-to-location,
sound spam, spectate, give-collectible, vehicle kick, teleport-to-warehouse,
start-activity, fake notifications, transaction error, and the three crash
events.

**The event-hash problem, and how it is solved.** YimMenu's `eRemoteEvent`
constants are joaat hashes compiled into the freemode script. They are *not*
portable and must not be copied on faith. I checked: all 42 of YimMenu's
constants were tested against the 1389 hashed entries in the leaked
`MP_Event_Enums.sch`, under both case conventions of joaat. **Zero matched.**
The leaked dev branch, the PC build YimMenu targets, and PS4 1.57 are three
different script revisions, and nothing licenses assuming any two agree.

So the hashes are **derived, not ported**. The mechanism is a `learn` mode on
the script-event filter: it reports `args[0]`, `args` count and the sender for
every script event received, without blocking anything. A session under normal
play establishes the benign baseline; a session with a known attack tool
identifies the hostile ones. Only hashes confirmed this way are added to the
block list, each recorded in the source with the evidence for it.

This is slower than pasting an enum, and it is the only honest way to do it. A
block list built from unverified constants would either do nothing (harmless but
dishonest) or drop legitimate script events (a bug that presents as random
mission failures). The learn mode is also permanently useful: it is how any
future event gets identified.

**Per-event filters.** Beyond script events, YimMenu filters ~17 specific event
ids. Each is reached by taking the event's vtable — the factory table at
`mgr + 242648` gives the registered function per id, and the vtable is stamped
as its first member write — then detouring that vtable's `Decide` (slot 8) or
`Handle` (slot 7). The 81-event catalogue in `analysis/NET_EVENT_CATALOG.md`
provides the id and registered function for every one.

Prioritised for LSO, in this order: `WEAPON_DAMAGE_EVENT` (weapon validity via
`g_WeaponInfoArray`), `EXPLOSION_EVENT`, `NETWORK_CLEAR_PED_TASKS_EVENT`,
`RAGDOLL_REQUEST_EVENT`, `GIVE_CONTROL_EVENT`, `NETWORK_PLAY_SOUND_EVENT`,
`KICK_VOTES_EVENT`, `SCRIPT_WORLD_STATE_EVENT`, `REQUEST_CONTROL_EVENT`,
`REMOVE_WEAPON_EVENT`, `GIVE_WEAPON_EVENT`, `REPORT_CASH_SPAWN_EVENT`,
`REPORT_MYSELF_EVENT`, `SCRIPT_ENTITY_STATE_CHANGE_EVENT`,
`ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT`, `DOOR_BREAK_EVENT`,
`CHANGE_RADIO_STATION_EVENT`.

Each needs its event struct's field offsets confirmed. That is per-event work,
independent, and safe to do in `Log` mode.

## Tier 3 — clone and object layer

`CNetworkObjectMgr::ReceivedCloneCreate`, `ReceivedCloneSync` and
`ReceivedCloneRemove` are not yet located; `g_NetworkObjectMgr` (`0x385F5E0`)
and `GetNetworkObject` (`0x1EE5C80`) are, and the object manager's vtable is
reachable through the former, which is where the search starts.

The filters themselves port cleanly, because they read the netObject fields that
are already live-verified:

- object type outside `AUTOMOBILE … TRAIN` → reject
- an existing object whose type disagrees with the claimed type → reject
- an object id matching the local player's ped → reject (player-ped deletion and
  reverse-sync are the same attack seen from two sides)
- a `PLAYER` object whose owner is not the sender → reject
- per-player `clone_sync` / `clone_create` block bits → reject
- vehicle creates when the vehicle allocator is nearly exhausted → reject

Also in this tier: `update_sync_tree` (reject when the object has no sync data),
`received_array_update`, and `receive_pickup`.

`received_array_update` is the one Tier 3 filter that does **not** port cleanly:
YimMenu's version reaches into freemode script locals by hardcoded index to undo
a beast-mode assignment and a remote wanted-level, and those indices are as
build-specific as the event hashes. Only its structural half — the freemode-state
`CLOSING` kick detection — ports. The script-local half is out of scope, and
`network_players` already offers no equivalent, so nothing regresses.

**Net message filters.** YimMenu's `receive_net_message` handles 17 message
types, most of which are R* session management. On LSO, the ones that matter are
the host-authority messages: `MsgKickPlayer`, `MsgRequestKickFromHost`,
`MsgScriptHostRequest`, `MsgScriptMigrateHost`, and `MsgNetTimeSync`. A private
server's host is still a player, and host-kick is a live attack. This is scoped
to those five, not the full set.

## Tier 4 — sync-node validator

`can_apply_data` is YimMenu's largest protection: ~60 sync-node types, each with
field-level validity checks. It is the deepest defence and the slowest to build.

The mechanism ports better than expected, because the piece I assumed would be
the blocker is already solved. YimMenu identifies a node by walking each sync
tree's node array and mapping array index → node name from a hardcoded per-tree
order list, then vtable → node id. The order list comes from the class
declaration order in the leaked source, which is build-independent. And all 13
`g_SyncTree_*` singletons are already resolved (`0x39B12B8` … `0x39B4E80`), so
the vtable→name map can be built at runtime exactly as YimMenu builds it.

What does not port for free is the **field offsets inside each node struct**.
PC is MSVC, PS4 is clang; the layouts are probably the same for these POD-ish
classes but that is an assumption, not a fact, and it must be checked per node
before that node's filter is trusted in `Enforce`.

The hook point is `netSyncTree::ApplyNodes` (`0x1E9D500`) or the `CanApplyData`
equivalent, to be confirmed during the tier.

This tier is sequenced last deliberately. It is the highest-volume work, it sits
in the hottest path in the whole design, and every one of its ~60 checks is
independently verifiable in `Log` mode — which means it can be built and landed
incrementally, node by node, without a long-lived branch.

`serialize_parachute_task` and `serialize_take_off_ped_variation_task` belong to
this tier; both are small, and the second needs its parachute model hash list
checked against 1.57 rather than copied.

## Deliberately not ported

Three protection hooks are dropped entirely:

| YimMenu hook | Why not |
|---|---|
| `handle_join_request` | The LSO server decides who joins. This filters a R* join handshake that is not in the path. |
| `update_presence_attribute` | Blocks R* presence attributes (`gstok`, `gsid`, `gshost`…) to defeat RID-joining. No R* presence on LSO. |
| `send_non_physical_player_data` | Bubble manipulation to keep a specific player out. Depends on the R* session bubble handshake. Revisit if LSO turns out to use it. |

Two are ported in part, as described in their tiers: `received_array_update`
(structural half only, not the freemode script-local half) and
`receive_net_message` (5 host-authority messages of 17).

One is ported with changed behaviour: `create_pool_item` becomes log-only rather
than YimMenu's `LOGF(FATAL)` — killing the process on a pool exhaustion is a
worse outcome than the exhaustion.

## Risks

**A filter crashes the game on the network thread.** The most likely failure and
the most disruptive. Mitigated structurally: `Log` mode before `Enforce`, and
filters restricted to pure reads. A crash in `Log` mode is still a crash, so
Tier 1 — testable without a session — is built first, which shakes out the
detour infrastructure before it is used anywhere hot.

**Detour count on a hot path.** The skill's own warning is that a hot hook
touching per-ped TLS is a plausible crash, and that at most one detour belongs
on a hot path at a time. Tier 4 in particular puts a detour in the sync path.
Mitigation: land tiers one at a time, verify each on console, and keep the mode
byte check as the first thing every hook does so a disabled filter costs one
predictable branch.

**Struct offsets differ from PC.** Assumed everywhere a YimMenu filter reads a
field. Mitigated by `Log` mode and by `ps4_verify_struct` against the live game
before any filter is trusted in `Enforce`.

**Verification needs an attacker.** Most filters only fire under attack. `Log`
mode plus normal play establishes the false-positive rate, which is the half
that matters most — a filter that never fires is useless, but a filter that
fires on legitimate traffic is actively harmful. Confirming true positives needs
a second console or a crafted sender, and is deferred rather than blocked on.

**Stale attachment during Frame4 verification.** After a game restart the old
attachment silently returns all-zero reads, which looks exactly like a wrong
offset. Re-attach and cross-check in-process before believing any offset is
wrong.

## Success criteria

1. Tier 1 installs and the game runs normally, single-player, with all guards in
   `Enforce`. No crash, no measurable frame cost.
2. The script-event learn mode logs `args[0]` for a full LSO session and the
   benign baseline is written down.
3. At least one script-event hash is confirmed hostile and blocked in `Enforce`,
   verified by the attack no longer landing.
4. Tier 3 clone filters run a full session in `Log` with zero reports during
   normal play — the false-positive check.
5. Per-player block bits demonstrably drop that player's events while leaving
   other players unaffected.
6. Every filter's mode persists across a game restart.

Tier 4 has no completion criterion in this spec: it lands node by node, each
node meeting criterion 4's standard on its own.
