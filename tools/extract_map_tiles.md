# Extracting the pause-map tiles

How `assets/web/map/tile_<row>_<col>.png` (6 files) was produced, so it can be
regenerated later without re-deriving anything.

## Source

PC copy of GTA V, read through the CodeWalker MCP server
(`mcp__codewalker__binary_to_xml`). At the time this was run,
`server_status` reported the game directory as:

```
C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto V Enhanced
```

(Gen9-Modus / Enhanced edition build. The RPF path below matched exactly;
if a future run is against the Legacy build and the path has moved, search
for `minimap_sea_0_0.ytd` inside `x64b.rpf` first.)

Six texture dictionaries, one per pause-map quadrant:

```
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_0_0.ytd
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_0_1.ytd
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_1_0.ytd
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_1_1.ytd
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_2_0.ytd
x64b.rpf\data\cdimages\scaleform_generic.rpf\minimap_sea_2_1.ytd
```

### Why `minimap_sea_*` and not `minimap_*`

`x64a.rpf\data\tune\minimap.ymt` sets `eBitmapForPause` to
`MM_BITMAP_VERSION_SEA`. That's the variant the pause map actually draws.
It's also the higher-detail set: each `minimap_sea_*` texture dictionary is
~190 KB against ~35 KB for the corresponding `minimap_*` one.

## Step 1: Export to DDS

For each `<row>_<col>` in `0_0, 0_1, 1_0, 1_1, 2_0, 2_1`:

```
binary_to_xml(
  inputPath     = "x64b.rpf\\data\\cdimages\\scaleform_generic.rpf\\minimap_sea_<row>_<col>.ytd",
  outputXmlPath = "E:\\Projects\\PS4\\InsulinEngine\\InsulinGTAV\\build\\maptiles\\minimap_sea_<row>_<col>.xml",
  textureFolder = "E:\\Projects\\PS4\\InsulinEngine\\InsulinGTAV\\build\\maptiles\\dds"
)
```

CodeWalker writes each DDS into its own subfolder named after the texture
(`build\maptiles\dds\minimap_sea_<row>_<col>\minimap_sea_<row>_<col>.dds`),
not flat into `dds\`, so any glob that reads them back needs to recurse
(`dds\**\*.dds`).

The written CWXML for each texture reports:

```
Width value="1024"
Height value="1024"
MipLevels value="1"
Format>D3DFMT_DXT5
```

This was cross-checked, not taken on faith: the four-character code at byte
offset 84 of each `.dds` file (the `DDS_PIXELFORMAT.dwFourCC` field) reads
`44 58 54 35` = ASCII `"DXT5"`, confirming BC3/DXT5 compression. Each file is
1,048,704 bytes (128-byte DDS header + 1024×1024×1 byte/px for BC3). Pillow
decodes DXT1/3/5 natively, so no `texconv` fallback was needed here — all six
came back as DXT5 and opened fine.

## Step 2: Convert DDS to PNG

```python
from PIL import Image
import glob, os, pathlib
out = pathlib.Path(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map')
out.mkdir(parents=True, exist_ok=True)
for f in sorted(glob.glob(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build\maptiles\dds\**\*.dds', recursive=True)):
    name = os.path.basename(f).lower()
    # minimap_sea_<row>_<col>.dds -> tile_<row>_<col>.png
    parts = name.replace('.dds', '').split('_')
    row, col = parts[-2], parts[-1]
    im = Image.open(f).convert('RGB')
    print(name, im.size, im.mode)
    im.save(out / f'tile_{row}_{col}.png', optimize=True)
```

All six decoded as 1024×1024 RGB.

## Step 3: Verify the row/column mapping

Stitched all six into a 2-wide-by-3-tall sheet (`assets/map_check.png`,
deleted afterwards) and looked at it:

```python
from PIL import Image
import pathlib
d = pathlib.Path(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map')
t = Image.open(d / 'tile_0_0.png')
w, h = t.size
sheet = Image.new('RGB', (w * 2, h * 3))
for r in range(3):
    for c in range(2):
        sheet.paste(Image.open(d / f'tile_{r}_{c}.png'), (c * w, r * h))
sheet.save(d.parent.parent / 'map_check.png')
```

Result: the sheet reads as Los Santos, correctly oriented — the dense city
street grid, the airport runway grid, and the port/industrial grid all sit
at the **bottom** (south); the sparser, mountainous terrain sits at the
**top** (north), with a small town grid near the top-right consistent with
Paleto Bay and a large lake roughly mid-sheet consistent with the Alamo Sea.
The whole landmass is ringed by a light coastline band, as expected for the
"sea" bitmap variant, which stylises the coast on every side rather than
just the west. **No row/column swap was needed** — the deduced indexing
(`minimap_sea_<row>_<col>` mapping directly to `tile_<row>_<col>` with row
running north-to-south and column west-to-east) was correct as extracted.

## Step 4: Projection constants

From `x64a.rpf\data\tune\minimap.ymt`:

| Constant          | Value          |
|-------------------|----------------|
| `iBitmapTilesX`   | 2              |
| `iBitmapTilesY`   | 3              |
| `vBitmapTileSize`  | 4500 × 4500    |
| `vBitmapStart`    | -4140, 8400    |

Resulting world bounds covered by the 2×3 tile grid:

- X: `-4140 .. 4860` (i.e. `-4140 + 2 * 4500`)
- Y: `8400 .. -5100` (i.e. `8400 - 3 * 4500`; Y decreases going south/down
  the sheet, consistent with row 0 being the northernmost row)

Each tile therefore covers a 4500×4500 game-unit square, and the export
produced tiles at **1024×1024 pixels**, so 1 tile pixel ≈ 4.3945 game units
(4500 / 1024) along both axes.

Tile `<row>,<col>` covers:
- X: `vBitmapStart.x + col * 4500 .. vBitmapStart.x + (col + 1) * 4500`
- Y: `vBitmapStart.y - row * 4500 .. vBitmapStart.y - (row + 1) * 4500`

## Step 5: Final PNG encoding — fitting the 256 KB response buffer

`src/net/conn.cpp:15` sets `k_io_cap = 256 * 1024` (262,144 bytes), and the
static-file read buffer at `conn.cpp:143` is actually `k_io_cap - 1024` =
**261,120 bytes** — any file that doesn't fit entirely in that buffer gets a
`413 file too large` instead of being streamed (`conn.cpp:143-150`).

The straight `.convert('RGB')` PNGs from Step 2 all exceeded this, some by
a wide margin:

| Tile | RGB PNG size |
|------|-------------:|
| tile_0_0.png | 270,086 B |
| tile_0_1.png | 318,316 B |
| tile_1_0.png | 457,073 B |
| tile_1_1.png | 418,201 B |
| tile_2_0.png | 377,802 B |
| tile_2_1.png | 271,809 B |

Every one of the six tripped the limit — none would have been served. Each
tile only has ~2,100-2,350 unique colors (checked with
`Image.getcolors()`), so instead of downscaling the pixel dimensions (which
would have coarsened the map-click-to-world-coordinate mapping the next
task depends on), the tiles were re-encoded as adaptive 256-color palette
PNGs at the **same 1024×1024 resolution**:

```python
from PIL import Image
import glob, os
for f in sorted(glob.glob(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map\*.png')):
    im = Image.open(f).convert('RGB')
    pal = im.convert('P', palette=Image.ADAPTIVE, colors=256)
    pal.save(f, optimize=True)
```

A cropped side-by-side comparison of the RGB and palette-256 versions showed
no perceptible difference. Final sizes, all comfortably under the 261,120
byte limit:

| Tile | Palette-256 PNG size |
|------|---------------------:|
| tile_0_0.png | 135,136 B |
| tile_0_1.png | 137,267 B |
| tile_1_0.png | 202,260 B |
| tile_1_1.png | 200,544 B |
| tile_2_0.png | 162,357 B |
| tile_2_1.png | 135,268 B |

## Step 6: Deploy

```python
import ftplib, io, glob, os
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
try: f.mkd('/data/Ozark/web/map')
except Exception as e: print('mkd', e)
for p in sorted(glob.glob(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map\*.png')):
    data = open(p, 'rb').read()
    f.storbinary('STOR /data/Ozark/web/map/' + os.path.basename(p), io.BytesIO(data))
    print(os.path.basename(p), len(data))
f.retrlines('LIST /data/Ozark/web/map')
f.quit()
```

Then confirmed over HTTP:

```bash
curl -sI "http://10.10.10.236:8080/map/tile_0_0.png?pin=<PIN>"
```

Expect `200` and `Content-Type: image/png`, with `Content-Length` matching
the local file size exactly.

### Getting the PIN without touching the game

The PIN is generated randomly each time the companion server is started
in-game (`companion.cpp: make_pin()`), so it can't be hard-coded here. It
doesn't require running the game or a debugger to read it, though: `notify()`
in `platform/log.cpp` logs every on-screen notification — including
`"Companion server on :8080, PIN <nnnn>"` — to `/data/Ozark/insulingtav.log`,
which is a plain file on the console's filesystem. Pulling it over the same
FTP connection used for the deploy is enough:

```python
import ftplib, io
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
buf = io.BytesIO()
f.retrbinary('RETR /data/Ozark/insulingtav.log', buf.write)
print(buf.getvalue().decode('utf-8', errors='replace'))
f.quit()
```

### Actual result (2026-09-07)

The console had a session already running (log build tag `map-2`) with the
companion server started, PIN `9129` recovered from the log as above. All
six tiles came back `200 OK`, `Content-Type: image/png`, with
`Content-Length` matching the deployed file size exactly:

| Tile | Content-Length | Under 261,120 B limit? |
|------|----------------:|:-----------------------:|
| tile_0_0.png | 135,136 | yes |
| tile_0_1.png | 137,267 | yes |
| tile_1_0.png | 202,260 | yes |
| tile_1_1.png | 200,544 | yes |
| tile_2_0.png | 162,357 | yes |
| tile_2_1.png | 135,268 | yes |
