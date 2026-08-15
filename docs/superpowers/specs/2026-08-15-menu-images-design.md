# Menu images: pick a picture for the header and the background

Lets a user drop a PNG, JPG or GIF into a folder on the console and choose it as
the menu's header or background — animated or static — without a PC in the loop.

## What exists today

The machinery is already there and unused.

`global/ui_vars.h` declares **14** `menu_texture` slots (header, background,
scroller, footer, tooltip, panels, notify, …), each carrying an `m_enabled` flag
and an `m_texture` name. `renderer::get_texture()` reads exactly those. Two
things stop it working:

- `menu::textures::get_list()` is never populated. The port shipped
  `textures.{h,cpp}` as a stub whose comment says custom-YTD streaming is
  unsolved on console. That stopped being true when the animated-texture work
  landed `rage::gfx::texture_dictionary`, which injects a real dictionary into
  the txd store. The comment is stale; the stub was never revisited.
- `get_texture()` returns the dictionary name `"ozarktextures"`. The dictionary
  this project actually creates is called `"insulin"`. Nothing is registered
  under the former name, so even a populated list would resolve to nothing.

Because the lookup always misses, `get_texture()` falls through to the
`"randomha"` sentinel and the background is drawn as a solid colour quad.

On the loading side, `rage::gfx::create_texture_from_file` goes through
`grcTextureFactory::Create(filename, params)`, which calls `grcImage::Load`
(eboot `0x19CF830`). That reads four bytes and requires the `DDS ` magic —
PNG and JPG are rejected and produce a magenta/green checkerboard. This is why
`tools/gif2frames.ps1` exists and why converting has been a PC-side step.

## Scope

Two slots: **header** and **background**. The other twelve keep their solid
colours. Both slots accept a still image or an animated GIF.

Not in scope: the remaining twelve slots, DXT compression, replacing game
textures, and any path that bypasses the file-based engine loader.

## Approach

`stb_image` is already in the OpenOrbis toolchain at
`include/stb/stb_image.h`, with `stb_image_resize.h` beside it. It decodes to
RGBA8; the engine wants DDS on disk. So:

```
/data/Ozark/images/foo.gif
        │  the user picks it in the menu
        ▼
  stb_image  ──►  N frames of RGBA8
        │  stb_image_resize ──► 512x1024 (background) / 512x128 (header)
        ▼
/data/Ozark/images/.cache/foo/frame_000.dds …  + frames.json   (animated)
/data/Ozark/images/.cache/foo.dds                              (still)
        ▼
  rage::gfx::menu_textures().add(...) + commit()
        ▼
  m_header.m_texture = "foo";  m_header.m_enabled = true
        ▼
  renderer::get_texture() ──► { "insulin", "foo" } ──► draw_sprite
```

Conversion runs **when a picture is picked**, not at boot and not on a sweep of
the folder. Two reasons: `menu::build()` is where this project has had two
crashes and is the last place to start an image decoder; and converting a
folder full of GIFs that nobody selected is work done for nothing. The cache
makes it a once-per-image cost.

Rejected alternatives: a "Convert All" button (converts what you never use);
decoding into memory each boot (the engine takes a filename, so files get
written anyway — just repeatedly).

## Components

**`src/util/image/decode.{h,cpp}`** — the stb_image wrapper. Returns width,
height, frame count, per-frame delays and the RGBA8 pixels. Limited to
`STBI_ONLY_PNG`, `STBI_ONLY_JPEG`, `STBI_ONLY_GIF`, `STBI_ONLY_BMP` so the
binary does not grow by decoders nothing calls. `STB_IMAGE_IMPLEMENTATION` is
defined in exactly one translation unit.

**The first implementation step is to confirm this copy of stb_image exposes
`stbi_load_gif_from_memory`.** Older versions do not, and without it animated
GIF is a different design. Nothing else should be built until that is known.

**`src/util/image/dds_write.{h,cpp}`** — writes one uncompressed B8G8R8A8
frame: 4-byte magic, the 124-byte `DDS_HEADER`, a 20-byte `DDS_HEADER_DXT10`,
then top-down pixel rows. The byte layout is transcribed from
`tools/gif2frames.ps1`, which documents it and is proven on this console — not
re-derived. A wrong header byte does not produce an error; it produces the
checkerboard, and the search then starts in the wrong place.

**`src/util/image/scale.h`** — the target-size arithmetic: given a source size
and a slot maximum, produce the destination size with the aspect ratio kept.
Pure, and host-tested.

**`src/menu/base/util/menu_images.{h,cpp}`** — orchestration. Lists source
images, reports what is cached, converts on demand, registers the result with
`rage::gfx::menu_textures()`, and assigns the slot.

**`src/menu/base/submenus/settings_images.{h,cpp}`** — the picker, under
Settings → Themes → Menu Images, with a Header entry and a Background entry.
Each lists the images in `/data/Ozark/images/` plus "None", marking which are
already cached so the user can tell a instant choice from one that will pause.

## Two repairs this depends on

Both are prerequisites, not side quests:

- `menu::textures::get_list()` gets populated from the cache directory, so
  `get_texture()` can find a name.
- `get_texture()` returns `{"insulin", name}`. The `"ozarktextures"` string
  appears twice - once as that return value (`renderer.cpp:27`) and once as a
  special case telling `draw_sprite` not to stream-request it
  (`renderer.cpp:287`). Both go; `rage::gfx::is_custom_dict` already covers the
  second case correctly for the real dictionary name.

## The one renderer change

The header's animation lookup is hardcoded to the name `"banner"` in two places
- `renderer.cpp:53` for the header sprite and `:115` in the title path - and the
background has no animation path at all. Supporting "animated or static, either
slot" means turning that into a per-slot lookup and giving the background the
same branch the header already has. This is the only file in the renderer that
changes, and the change is a generalisation of an existing branch rather than a
new drawing path.

## Sizing

Target maxima are **512×1024** for the background and **512×128** for the
header, aspect ratio preserved, downscale only — a small image is never blown
up.

Those numbers come from the geometry: at 1080p the menu background covers about
420×700 pixels and the header about 420×86. The maxima sit comfortably above
that so other resolutions have room, while bounding a 60-frame GIF to roughly
120 MB of intermediate pixels rather than however much the source happens to
be. Frames are written out one at a time, so only two canvases are live at once.

## Persistence

The slot assignment goes into the **theme** system, not `config.json`.
`theme.cpp` already serialises colours, fonts and positions, and its own header
comment names per-slot textures as the intended follow-up. Putting them there
makes a theme complete: colours and pictures in one file.

Applying a theme therefore has to convert-or-load the referenced images. A theme
naming an image that is not on this console leaves that slot on its default and
says so, rather than failing to load.

## Failure modes

- **Decode fails** (corrupt file, unsupported format): named notification, slot
  unchanged. Follows `texture_dictionary::add`, which already rejects PNG with a
  named error rather than skipping it silently.
- **Cache cannot be written**: named notification, slot unchanged.
- **A cached frame is missing or unreadable at load**: the slot falls back to
  the sentinel — a solid quad, which is what it draws today.
- **A GIF with one frame** is treated as a still image, not a one-frame
  animation.
- **The source is newer than its cache**: reconverted. Cache staleness is
  compared by modification time.

## Testing

Host tests, built as the existing four are
(`clang++ -std=c++17 -I src tests/<name>.cpp -o build/<name>.exe`):

- `tests/dds_write_test.cpp` — the header bytes of a written frame against the
  known-good layout, including the DX10 block and the top-down row order.
- `tests/image_scale_test.cpp` — aspect ratio preserved; a source smaller than
  the maximum is untouched; extreme ratios do not produce a zero dimension.

On console:

1. Drop a PNG in `/data/Ozark/images/`, pick it as Background — it appears.
2. Pick an animated GIF as Header — it plays.
3. Pick an animated GIF as Background — it plays, and the frame rate holds.
4. Restart — both selections come back.
5. Pick "None" — the slot returns to its solid colour.
6. Drop a deliberately broken file and pick it — a named message, and the menu
   keeps drawing.

## Out of scope

- The other twelve texture slots. The design is generic enough that adding them
  later is a table entry each, but the menu surface is not worth it yet.
- Replacing or swapping the game's own textures.
- Any in-memory path around `grcImage::Load`, which would need
  `grcImage::Create` reversed and buys nothing here.

## Known limitations, as built

Four things this shipped with. None is a bug to hunt; each is a decision, and
the reason is here so it does not get rediscovered as a mystery.

**Re-picking a slot strands video memory.** `texture_dictionary::add_texture`
replaces an entry with `m_entries[i].tex = tex;` and never releases the old
`grcTexture` — there is no release path anywhere in this codebase, and building
one means reversing the engine's texture destructor. Re-picking a background
therefore strands roughly 33 MB (16 frames at up to 512×1024×4); the header
about 4 MB. Re-picking the picture *already* in a slot is free — that case
returns early. Replacing one logs what it cost. A session that cycles through
ten backgrounds strands a few hundred MB until the game restarts.

**Animations cap at 16 frames.** `menu::animation::k_max_frames`. A longer GIF
is truncated and the user is told, so a 26-frame source loops visibly short.
At the size maxima, 16 frames of background is 33 MB resident and the header
4 MB.

**Sources are refused above 4096 in either dimension.** `stb_image` decodes
every GIF frame at full source resolution in one allocation, before the frame
cap and before downscaling — a 1080p 60-frame GIF is roughly 500 MB inside the
game process. The dimension guard bounds the common case. Frame count cannot be
known before decoding, so a small-but-very-long GIF is still unbounded.

**The shared dictionary holds 64 textures.** Animation frames re-register under
fixed per-slot names and stay capped at 32 for both slots, but each distinct
picture applied adds one permanent still entry — so roughly 32 pictures per
session. Past that, applying fails with a named error rather than silently
drawing the checkerboard.
