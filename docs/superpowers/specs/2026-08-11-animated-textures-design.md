# Animated Textures — Design

**Date:** 2026-08-11
**Status:** approved, ready for an implementation plan
**Supersedes the open questions in** `docs/superpowers/plans/2026-08-10-animated-gif-textures.md`
(that document remains as the feasibility study it was; every question it left open is
answered here).

## Goal

A reusable building block that plays a frame sequence anywhere the menu can draw a
sprite. The menu header is its first consumer: the static custom banner becomes an
animated one. Source material is GIFs, converted on the PC into frames.

## Decisions

**Generic block, not a banner feature.** `animated_texture` holds frames and timing and
nothing else. The header is one consumer; icons, panels or overlays can be others without
touching the type.

**Frames on disk, not a runtime GIF decoder (v1).** The whole texture path today is
file-based: `grcTextureGNM(filename)` does `grcImage::Load` + `Create` + `Copy`
internally (`src/rage/gfx.cpp:90`). Decoding a `.gif` in the plugin would additionally
require two engine entry points that no code here has yet — a `grcImage` built from a RAM
buffer, and a `grcTexture` built from that image. That is reverse engineering with an open
outcome standing between us and any animation at all. It also saves less than it appears:
a decoder still needs N textures resident, one per frame. Deferred, not abandoned — see
*Follow-up*.

**One frame = one texture; no sprite atlas.** An atlas (all frames in one texture, UV
window scrolling) would mean one file load instead of sixteen, but `natives.h` exposes
exactly one `draw_sprite` (RVA `0x9CCCD0`) and it takes no UV parameters. An atlas needs a
`DRAW_SPRITE_*_WITH_UV` native, IDA-verified against the v1.57 eboot — the same open-ended
RE this design is avoiding, to save ~1.5 MB.

**Sixteen frames per animation.** Enough for a recognisable loop at banner size, small
enough that an animation plus the other custom textures fit in one dictionary (the store
dictionary holds 64 entries, `gfx.cpp:129`) and cheap enough to ignore load time.

## Architecture

### `menu::animated_texture` — `src/menu/base/util/animated_texture.{h,cpp}`

State: dictionary name, a list of `{texture name, delay ms}`, a time accumulator, the
current index, a loop flag.

```
add_frame(name, delay_ms)
advance(dt)                  // dt in seconds (global::ui::g_delta)
current() -> pair<dict,name> // feeds straight into draw_sprite
reset() / ready() / frame_count()
```

`advance` steps as many frames as are due, not one per call: after a frame drop of 100 ms
the animation catches up instead of falling into slow motion. The accumulator is taken
modulo the sequence's total duration first, so a long gap (menu closed, then opened)
cannot make it walk hundreds of frames.

With `loop` false, the sequence stops on its last frame and stays there; `advance` becomes
a no-op until `reset()`. `current()` keeps returning that frame, so a non-looping animation
degrades into a still image rather than disappearing.

**This type knows nothing about files.** That is the seam a future GIF decoder plugs into:
it would call `add_frame` with textures it created, and nothing in this type changes. No
loader interface, no format registry, no decoder stub is built now.

### `menu::animation` — same translation unit

- `load_from_dir(name, path)` — scans the directory, adds each frame to
  `rage::gfx::menu_textures()` under an explicit name, calls `commit()` once at the end,
  and fills an `animated_texture` with the delays. Direction `menu → rage`, as everywhere
  else in the tree. Returns `animated_texture*` — the registered instance, owned by the
  registry — or `nullptr` if nothing loadable was found.
- `get(name)` — the registered animation under that name, or `nullptr`. This is how a
  consumer reaches an animation it did not load itself.
- `update(dt)` — advances every registered animation. Called from `menu::tick()` next to
  `notify::update()` / `panels::update()`, so stepping (update) stays separate from
  drawing (render) and the renderer keeps no side effects.
- `header_asset()` — `get("banner")->current()` when that animation is loaded and ready,
  otherwise the static `{"insulin","logo"}`. The header animation is therefore the one
  registered under the name `banner`, loaded from `/data/insulin/anim/banner/`; no
  separate "is header" flag exists.

Registration is a function-local static vector (no `.init_array` on GoldHEN). Animations
live for the session; there is no unload path in v1 — the textures are pinned in the
store anyway (`AddRef` in `commit()`).

### Renderer

`renderer.cpp` asks `menu::animation::header_asset()` instead of naming
`{"insulin","logo"}` directly. The existing three-step fallback is preserved: animation →
static custom logo → the game/sentinel header.

## Data

```
/data/insulin/logo.dds            static custom textures (unchanged)
/data/insulin/anim/banner/        000.png … 015.png + frames.json
```

`frames.json`, parsed with the existing `tj` mini-JSON (as themes and language files are):

```json
{ "loop": true, "default_delay": 66,
  "frames": [ { "file": "000.png", "delay": 66 } ] }
```

Without the manifest the directory is scanned for `*.png` / `*.dds` in name order and every
frame gets 66 ms (≈ 15 fps), so a folder of PNGs alone is a valid animation.
Per-frame delays exist because GIFs genuinely vary them frame to frame; the conversion
script writes the real values.

Texture names in the dictionary are `banner_000 … banner_015` — assigned by the loader,
not derived from the file stem. Two animations both starting at `000` would otherwise
collide, and `commit()` drops colliding codes (`gfx.cpp:223`), i.e. frames would silently
go missing.

## Error handling

None of these are fatal; the menu keeps running.

| Case | Behaviour |
|---|---|
| Directory missing or empty | `ready()` stays false, header falls back to the static logo, one log line |
| A single frame fails to load | skip it, the rest of the sequence still plays |
| More than 16 frames | capped at 16 **with a warning in the log** — never a silent truncation |
| `delay` ≤ 0 or absent | `default_delay` |
| Menu closed for a long time | accumulator taken modulo total duration before stepping |
| Loaded with no session up | same as the existing custom-texture path: the factory singleton is null, `create_texture_from_file` logs and returns null |

## Memory

Sixteen 512×192 PNG frames are decompressed to RGBA by the engine, ≈ 6 MB — the default
path. The same frames as DXT5 DDS would be ≈ 1.5 MB; the loader accepts both, since it
takes file names from the manifest rather than filtering by extension. Converting to DDS
is a manual optimisation, not part of v1 tooling (see below).

## Tooling

`tools/gif2frames.ps1` — takes a `.gif`, writes the frame directory and `frames.json`
including the real per-frame delays. Without it the feature is not usable, so it is part
of the work.

It uses .NET `System.Drawing` directly (`FrameDimension.Time` for the frames, property item
`0x5100` for the delay table, which GIF stores in 1/100 s units) and emits PNG. Neither
ffmpeg nor ImageMagick is installed on the development machine and neither ships with
Windows, so depending on one would mean the script fails on first use; `System.Drawing` is
present with PowerShell 7 on Windows. Consequences: PNG output only — DDS conversion would
need a block compressor and is left as a manual step — and frames are written at the GIF's
own resolution, with resizing likewise manual.

## On-console acceptance

1. Copy a converted animation to `/data/insulin/anim/banner/`, load it from the menu.
2. The header animates smoothly at the intended rate.
3. Press the PS button and return: no crash. The extra textures go through the same
   suspend-time accounting pass the heap guard covers (`src/rage/heap_guard.h`).
4. With no animation present, the header still shows the static logo.

## Follow-up (not in v1)

Runtime `.gif` decoding — drop a `.gif` on `/data` and the menu animates it. Needs a
freestanding decoder (e.g. `stb_image`, which decodes animated GIF to RGBA and needs only
malloc/memcpy from SceLibcInternal) plus the two RE targets named above: `grcImage` from a
memory buffer, and `grcTextureFactory::Create(grcImage*)`. Verify both exist in the v1.57
IDB before committing to it. `animated_texture` is already the consumer they would feed;
no code is written for this now.

## References

- Texture pipeline: `src/rage/gfx.{h,cpp}`; RVAs and derivation in the file header
- Header draw + fallbacks: `src/menu/base/renderer.cpp`
- Frame time: `global::ui::g_delta`, refreshed in `menu::tick()` from `get_frame_time()`
- Suspend safety: `src/rage/heap_guard.h`
- Feasibility study: `docs/superpowers/plans/2026-08-10-animated-gif-textures.md`
