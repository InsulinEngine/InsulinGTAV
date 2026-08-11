# Plan: Animated GIF textures in the menu

**Status:** RESOLVED 2026-08-11 — every open question below is answered in
`docs/superpowers/specs/2026-08-11-animated-textures-design.md`, which is the document to
build from. Kept as the feasibility study it was. Verdict then and now: feasible, but
"load a GIF" really means "decode N frames + animate"; v1 does the decoding on the PC.

## The question
Can the menu load GIF files (for an animated banner/logo etc.)?

## How textures work today (facts, verified in this codebase)
- `src/rage/gfx.cpp` `create_texture_from_file(path)` calls the RAGE factory
  `grcTextureGNM(filename)` via `RVA_CREATE_FROM_FILE = 0x1A0FB10`. The engine does
  `grcImage::Load` + `Create` + `Copy` **internally**, keyed off the file.
- `texture_dictionary::add_directory()` filters on `.dds` and `.png` only.
- So the **engine** decides supported formats; PNG/DDS work because `grcImage::Load`
  supports them. The pipeline is **file-based** — there is currently NO path that
  builds a texture from raw in-memory pixels.
- Frame timing already exists: `native::get_frame_time()` / `global::ui::g_delta`.

## Key finding
1. **GIF is almost certainly NOT natively supported by RAGE `grcImage`** (RAGE
   typically does DDS/PNG/TGA/JPG/BMP, not GIF). Dropping a `.gif` in and expecting
   `Create()` to work will fail. → OPEN: verify in IDA what `grcImage::Load` dispatches
   on (extension/magic). Entry: the loader reached from `RVA_CREATE_FROM_FILE` chain in
   the v1.57 IDB (see memory `gta5-157-idb`, `E:\Projects\IDA\PS4\GTA5\eboot_named.i64`).
2. A single GIF frame gains nothing over PNG. The point is **animation**: decode all
   frames + per-frame delays, and swap the drawn texture each frame.

## Approach A — offline pre-split (RECOMMENDED for first version)
Convert the GIF on the PC into N PNG/DDS frames + a tiny manifest of delays; ship them;
add a small "animated texture" player in the menu that swaps between the already-loaded
frames by accumulated `g_delta`.
- **Pros:** reuses existing PNG pipeline 100%, ZERO new engine RE, robust, predictable.
- **Cons:** ships as frames, not a `.gif`.
- Steps:
  1. Tooling: `ffmpeg`/ImageMagick to split `foo.gif` -> `foo/000.png..NNN.png` + a
     `frames.txt` (or JSON) listing count + per-frame delay ms (GIF delays vary per frame).
  2. Load all frames into a `texture_dictionary` (existing `add`/`add_directory`).
  3. New menu helper `animated_texture`: holds frame handles + delays + accumulator;
     `advance(dt)` picks the current frame; expose `current()` to the draw call.
  4. Wire into wherever the banner/logo is drawn (see `rage::gfx::banner_ready()` /
     `menu_textures()` usage, and the renderer sprite draw).

## Approach B — runtime GIF decode (the "drop-a-.gif" version)
Bundle a freestanding GIF decoder (e.g. `stb_image` — decodes animated GIF to RGBA;
needs only malloc/memcpy from SceLibcInternal, fits the STL_FREESTANDING build), then
build textures from raw RGBA.
- **Blocker / extra work:** current code only has the **file**-based Create. Need a new
  engine entry point: `grcImage` from a memory buffer + `grcTexture` from that `grcImage`
  (or a direct raw-RGBA texture create). That is NEW reverse engineering beyond gfx.cpp
  today. RE targets to find in the IDB: `grcImage::Create`/from-memory and
  `grcTextureFactory::Create(grcImage*)`.
- **Pros:** true drop-in `.gif` at runtime. **Cons:** more RE + risk; do only if A isn't enough.

## Open questions to resolve before building (brainstorm these)
- Where does the GIF appear (banner? full-screen? small icon?) and at what size?
- How many frames / how big — memory + VRAM budget on PS4? (each frame is a full texture)
- Animated required, or is a single still acceptable for v1?
- A or B? (recommend A first.)

## Recommendation
Ship **Approach A** first (fast, low-risk, no new RE). Escalate to **B** only if
arbitrary runtime `.gif` loading becomes a real requirement. Before B, spend ~1 IDA
session confirming the `grcImage`-from-memory + texture-from-image path exists and its RVAs.

## Related
- Pipeline: `src/rage/gfx.cpp`, `src/rage/gfx.h`
- Frame timing: `src/game/game_thread.cpp` (`get_frame_time`), `global::ui::g_delta`
- IDB for RE: memory `gta5-157-idb` (author's raw addresses are +0x400000 rebased —
  subtract it; see memory `heap-guard-153-address`).
