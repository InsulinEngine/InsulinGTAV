# Companion server: a browser on the phone drives the console

Serves a small web page from inside the plugin, over the console's own network
interface. Two things it does on day one: replace any texture in the game with a
picture you upload, and teleport by clicking a map. Both exist to remove the same
pain — typing names and coordinates with a controller.

The console serves the page itself. No PC has to be running; a phone on the same
network only needs the console's IP.

## What exists today

More than the feature needs, which is why this is smaller than it looks.

**Image loading is solved.** `util/image/decode.cpp` decodes PNG, JPG and GIF
through stb; `scale.h` resizes; `dds_write.cpp` writes DDS. `menu_images.cpp`
chains them in `convert()` and caches the result per picture.

**Real game textures are already created by the engine, not by hand.**
`rage/gfx.cpp:184` calls the game's own factory:

| RVA | Symbol |
|---|---|
| `0x3B11CC8` | `grcTextureFactory::sm_Instance` |
| `0x1A0FB10` | `Create(const char* filename, params)` → `grcTextureGNM*` |

That matters more than it sounds. `LOADINGSCREEN_TEXTURES.md` §7 leaves one item
open — the byte offset of the GNM texture descriptor (T#) inside `grcTexture` —
and describes how to build or clone a descriptor by hand. **None of that is
needed here.** The factory builds the descriptor. The open item stays open, and
stays irrelevant to this work; it only matters for overwriting an existing
texture's pixels in place.

**The whole `fwTxdStore` surface is already wrapped** in `rage/gfx.cpp:19-25`:

| RVA | Symbol |
|---|---|
| `0x3E4B218` | `g_TxdStore` (embedded object; its address *is* `this`) |
| `0x25D2FC0` | `FindSlot(this, const char* name)` → idx or -1 |
| `0x25D3400` | `AddSlot(this, u32* pHash)` → idx |
| `0x1ECF0C0` | `GetPtr(this, int idx)` → `pgDictionary*` (redirect-aware) |
| `0x25D31C0` | `AddRef(this, int idx)` — pins the slot against streaming |
| `0x18AB110` | txd-name hash (the FindSlot/AddSlot key) |
| `0x1942AC0` | `atStringHash` — the pgDictionary entry key |

**pgDictionary is already walked read-only** in
`menu/base/submenus/vehicle_preview.cpp:44-133`, including the store-slot
validity check and the sorted-hash binary search. Layout, from the RE catalog:
`+0x10` parent, `+0x20` `u32* hashes` (ascending), `+0x28` `u16 count`,
`+0x30` `grcTexture** entries`.

**A dictionary entry is already replaced in place** in `menu_images.cpp:422+`,
for the plugin's own `insulin` dictionary.

**Sockets are available.** The OpenOrbis toolchain ships `libSceNet.so` and
`libxnet.a`. Nothing in the plugin uses them yet.

## The RPF finding this depends on

The texture catalogue needs the names of every texture in the game. Those live
inside the console's RPF archives, whose table of contents is encrypted. As of
2026-09-06 that is solved — see `GTA5_PS4_RE_CATALOG.md` Nachtrag 7 and
`LOADINGSCREEN_TEXTURES.md` §8 in the RE repo. The recipe, all of it read out of
the `dev_ng` source rather than guessed:

| Element | Value |
|---|---|
| Key id | `0x0FFEFFFF` = `AES_MULTIKEY_ID_GTA5_PS4` |
| Key table | `unique_multikey_gta5_ps4[101][32]`, in `data/multi_keys_AESo.c` |
| Cipher | AES-256 ECB, **one** pass, over `size & ~15` |
| Selector, TOC and name heap | `(joaat(rpf basename) + archive size) % 101` |
| Selector, per entry | `(joaat(entry name) + uncompressed size) % 101` |

Without this the catalogue would have to be approximated from a PC install. With
it, the catalogue is exactly what is on the console.

## Scope

In:

- HTTP server inside the plugin, off by default, toggled from the menu.
- Static hosting of a page, the catalogue file and map tiles from `/data`.
- Upload a picture and bind it to any `(txd, texture)` pair in the game.
- A registry that survives a restart and reapplies when a dictionary loads.
- A map with click-to-teleport and a live player marker.
- Two PC-side tools: an RPF reader and a catalogue builder.

Out:

- Any write to another player's session. This is a local tool.
- Repacking archives back into `/data/app` (Route B). Now possible, not wanted here.
- Editing textures in the browser beyond picking a file.
- Authentication beyond a PIN on a home network.

## Approach

Three decisions carry the design.

**The plugin never parses the catalogue.** It will hold tens of thousands of
entries. A JSON parser for that under the mini-STL is a liability with no
benefit. The plugin serves `catalog.json` as opaque bytes; search, filter and
preview happen in the browser. What reaches the console is one pair of short
names.

**Names go over the wire, not hashes.** The browser sends
`{"txd":"frontend","tex":"gtav_logo"}`. The console hashes them with the game's
own functions. The alternative — sending hashes — would require a hash-to-name
table on the console for the UI to be useful, and there is no reason to carry
one.

**Two requests instead of multipart.** A `multipart/form-data` parser working on
raw byte buffers is avoidable risk:

```
PUT  /api/upload/<id>     raw image bytes in the body
POST /api/swap            {"txd":"...","tex":"...","id":"..."}
```

### Thread split

This is where a feature like this kills the game, so it is stated as a rule
rather than a guideline.

| Thread | May do | May never do |
|---|---|---|
| HTTP | parse headers, check magic bytes, enforce size caps, write files under `/data`, push a job | touch any engine structure, call any native |
| Game (frame hook) | everything else | block |

The handover cannot use a lambda. `game::run_on_game_thread` takes a bare
`void(*)()` with no capture, and `stl::function` caps captures at 64 bytes —
smaller than a path. So the queue is explicit: a fixed-size ring of job structs
guarded by a spinlock, drained by the companion submenu's `feature_update()`,
which already runs once per frame and is already gated on `game::player_valid()`
upstream. No new scheduling machinery.

The server thread is created with an explicit `scePthreadAttr` and a set stack
size. A `NULL` attr gives a tiny stack, and a function with large locals faults
on entry with no useful log.

### The mini-STL trap, named early

`stl::string` is a fixed 128-byte buffer that truncates silently. An HTTP request
line plus headers is routinely longer. The server therefore works on raw byte
buffers with explicit lengths throughout, and `stl::string` appears nowhere in
the parsing path. Getting this wrong produces failures that look like network
problems and are not.

## Components

New in the plugin:

| File | Responsibility |
|---|---|
| `net/server.cpp` | listener thread, accept loop, connection lifetime |
| `net/http.cpp` | request line and header parsing on raw buffers, response writing |
| `net/routes.cpp` | static files, `/api/*`, upload sink |
| `net/jobs.h` | fixed job ring, spinlock, POD job structs |
| `texture/swap.cpp` | store lookup, dictionary entry replacement, restore |
| `texture/registry.cpp` | active and pending swaps, `swaps.json`, reapply |
| `menu/base/submenus/companion.cpp` | on/off switch, port, PIN, status, job drain |

New in the RE repo:

| File | Responsibility |
|---|---|
| `tools/ps4_rpf.py` | RPF7 reader per the table above, recursive, per-entry decrypt |
| `tools/build_catalog.py` | walk archives, read `.otd` via the existing `tools/otd_tool.py`, emit `catalog.json` and map tiles |

`catalog.json` is one array, one object per texture, short keys because the file
is large and shipped whole to the browser:

```json
{"d":"frontend","n":"gtav_logo","w":512,"h":512,"f":"R8G8B8A8","s":"ps4"}
```

`d` dictionary, `n` texture, `w`/`h` size, `f` format, `s` source — `ps4` for an
entry read out of the console dump, so the page can mark it console-verified.

On disk on the console:

```
/data/GoldHEN/insulin/web/       page, catalog.json, map tiles
/data/GoldHEN/insulin/textures/  uploaded pictures
/data/GoldHEN/insulin/swaps.json registry
```

## Texture swap: the actual sequence

On the game thread, per job:

1. `convert()` the uploaded file to a cached DDS — existing code.
2. `create_texture_from_file(dds)` → `grcTexture*` — existing code.
3. `FindSlot(txd)`; `-1` means not loaded — not an error, see below.
4. `GetPtr(idx)` → `pgDictionary*`; check the slot's validity flag first, as
   `vehicle_preview.cpp` already does.
5. Binary search `hashes[]` for `atStringHash(tex)`. A miss means the texture is
   not in that dictionary; report it back.
6. `AddRef(idx)` so streaming cannot evict the dictionary under us.
7. Remember `entries[i]`, then write our pointer into it.

Restoring writes the remembered pointer back.

**One texture per target, ever.** `texture_dictionary::add_texture` never
releases what it displaces, and `rage_alloc` has no free — this is documented at
`menu_images.cpp:464` and is why the existing menu warns about stranded video
memory. We cannot give memory back, so the registry refuses to create a second
texture for a target that already has one: reapplying reuses it.

## Failure modes

| Situation | Behaviour |
|---|---|
| Dictionary not loaded (`FindSlot` = -1) | swap stays *pending*; one pointer compare per active swap per second detects the load and applies. The browser shows "waiting for `loadingscreen0`". |
| Texture name not in that dictionary | rejected immediately, naming the texture, not silently dropped |
| Dictionary rebuilt after a stream-out | the same pointer compare notices and reapplies |
| Upload not a known image type | rejected on the HTTP thread by magic bytes, before anything is written |
| Upload too large | rejected against a fixed cap; buffers never grow |
| Job ring full | request answered with "busy" rather than blocking the HTTP thread |
| Server enabled during load | it is not — the switch is gated on `player_valid()` like every other feature |

## Access control

Off by default, and the switch lives in the menu. Default port **8080**,
changeable in the menu and persisted like any other setting. When on, the menu
shows the port and a four-digit PIN generated at enable time; every request
carries it as `X-Insulin-Pin`, and anything without it gets a `401` before the
body is read. This is not a defence against an attacker on the network; it
prevents another device on the same WLAN from stumbling in.

One connection at a time, fixed buffers, and a hard cap per request — 16 MB for
an upload, 8 KB for headers. A request that exceeds either is closed, not
buffered.

## Map and teleport

Tiles come out of the same offline chain as the catalogue and are served
statically. The world-to-map transform is not guessed: two known points are
checked live against the player position read from `CPedFactory → +0x08`, and the
factor and offset are derived from those.

```
GET  /api/state     {x, y, z, heading}    plain memory reads, no natives
POST /api/teleport  {"x":..,"y":..,"ground":true}
```

Ground height and the move itself go through the existing teleport submenu —
fade, resolve ground, wait for collision. That path is built and tested; this
only triggers it.

## Build order

Each step ships and is testable on its own.

1. **Server serves a static page.** No engine contact at all. If the game
   survives this, the thread split is right. This is the step that proves the
   riskiest part of the feature.
2. **`GET /api/state`.** Reads only.
3. **Map with click-to-teleport.** First write path, through existing code.
4. **Texture swap.** Catalogue, upload, registry, reapply.

## Testing

- Step 1 runs for an extended session with the page open and reloading, watching
  the kernel log at `10.10.10.236:3232` for anything prefixed `IGV`.
- Every job logs a stage marker before and after, in the `stage()` style already
  used in `menu_images.cpp`, so the last line names the step that died.
- The build tag in `module_start` is bumped on every deploy; `__TIME__` alone is
  unreliable because only the recompiled TU changes.
- Texture swaps are verified against `gtav_logo` first: it is 512×512 RGBA, its
  dictionary (`platform:/textures/frontend`, hash `0xE703E6C4`) was confirmed
  live in the running process on 2026-09-06, and the result is visible
  immediately in the pause menu.
- A crash that disappears after a redeploy with no code change is treated as an
  open bug, not a fix.

## Known limitations, as built

- Video memory for a replaced texture is never reclaimed. Bounded by one texture
  per target, not by session length.
- The catalogue is a snapshot of the dump on disk. A texture the game has but the
  dump does not will not be listed; the console-side lookup remains the
  authority.
- No TLS. Home network only.
- The T# offset from `LOADINGSCREEN_TEXTURES.md` §7 remains unmeasured. Nothing
  here needs it.
