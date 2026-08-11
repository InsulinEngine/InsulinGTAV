<#
.SYNOPSIS
    Converts an animated GIF into the frame directory the InsulinGTAV menu loads.

.DESCRIPTION
    Writes 000.dds..NNN.dds plus frames.json (per-frame delays in ms) into -OutDir.
    Copy that directory to /data/insulin/anim/<name>/ on the console.

    DDS is not a preference, it is the only thing that loads. The engine's
    grcImage::Load (eboot 0x19CF830) reads four bytes and requires the 'DDS ' magic;
    a PNG fails there, and its caller quietly substitutes a 32x32 magenta/green
    checkerboard that the texture factory then wraps in a perfectly valid texture.
    The result draws as stripes with no error anywhere.

    Frames are written uncompressed as B8G8R8A8 (DXGI 87) behind a DX10 header --
    the loader maps that to its internal format 11 and keeps the alpha channel.
    Uncompressed costs ~8x the bytes of BC1 but needs no block compressor, and the
    16-frame cap keeps a banner in the low megabytes.

    Uses .NET System.Drawing rather than ffmpeg or ImageMagick: neither is installed
    on the dev machine and neither ships with Windows, so depending on one would make
    this script fail on first use. Frames keep the GIF's own resolution.

    The menu plays at most 16 frames, so longer GIFs are sampled down evenly and the
    delays of dropped frames are added to the frame that is kept -- total duration and
    playback speed are preserved, smoothness is not.

.EXAMPLE
    pwsh -File tools/gif2frames.ps1 -Gif banner.gif -OutDir out/banner
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Gif,
    [Parameter(Mandatory = $true)] [string] $OutDir,
    [int]  $MaxFrames    = 16,
    [int]  $DefaultDelay = 66,
    [bool] $Loop         = $true
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Gif)) { throw "GIF not found: $Gif" }
Add-Type -AssemblyName System.Drawing

# Write one uncompressed B8G8R8A8 DDS. Layout is the standard 4-byte magic, the
# 124-byte DDS_HEADER, a 20-byte DDS_HEADER_DXT10, then top-down pixel rows.
# Field offsets were checked against the loader's own stack reads: it takes
# ddspf.dwSize from header offset 72 and dwFourCC from 80.
function Write-Dds {
    param(
        [Parameter(Mandatory = $true)] [System.Drawing.Bitmap] $Bitmap,
        [Parameter(Mandatory = $true)] [string] $Path
    )

    $w = $Bitmap.Width
    $h = $Bitmap.Height

    # Format32bppArgb is B,G,R,A in memory on a little-endian host, which is
    # exactly DXGI_FORMAT_B8G8R8A8_UNORM -- no channel swizzle needed. Rows are
    # copied one at a time because Stride may exceed w*4 for alignment.
    $pixels = New-Object byte[] ($w * $h * 4)
    $rect = New-Object System.Drawing.Rectangle 0, 0, $w, $h
    $data = $Bitmap.LockBits($rect, [System.Drawing.Imaging.ImageLockMode]::ReadOnly,
                             [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        for ($y = 0; $y -lt $h; $y++) {
            [System.Runtime.InteropServices.Marshal]::Copy(
                [IntPtr]::Add($data.Scan0, $y * $data.Stride), $pixels, $y * $w * 4, $w * 4)
        }
    } finally { $Bitmap.UnlockBits($data) }

    $fs = [System.IO.File]::Create($Path)
    $bw = New-Object System.IO.BinaryWriter $fs
    try {
        $bw.Write([System.Text.Encoding]::ASCII.GetBytes('DDS '))

        $bw.Write([uint32]124)          # dwSize
        $bw.Write([uint32]0x100F)       # CAPS | HEIGHT | WIDTH | PITCH | PIXELFORMAT
        $bw.Write([uint32]$h)
        $bw.Write([uint32]$w)
        $bw.Write([uint32]($w * 4))     # dwPitchOrLinearSize
        $bw.Write([uint32]0)            # dwDepth
        # dwMipMapCount stays 0 and DDSD_MIPMAPCOUNT stays unset, which the loader
        # reads as a single level; claiming more levels than the dimensions allow
        # is one of the ways it bails out to the checkerboard.
        $bw.Write([uint32]0)
        1..11 | ForEach-Object { $bw.Write([uint32]0) }   # dwReserved1

        $bw.Write([uint32]32)           # ddspf.dwSize
        $bw.Write([uint32]4)            # ddspf.dwFlags = DDPF_FOURCC
        $bw.Write([System.Text.Encoding]::ASCII.GetBytes('DX10'))
        1..5 | ForEach-Object { $bw.Write([uint32]0) }    # bit count + RGBA masks

        $bw.Write([uint32]0x1000)       # dwCaps = DDSCAPS_TEXTURE
        1..4 | ForEach-Object { $bw.Write([uint32]0) }    # dwCaps2..4, dwReserved2

        $bw.Write([uint32]87)           # dxgiFormat = B8G8R8A8_UNORM
        $bw.Write([uint32]3)            # resourceDimension = TEXTURE2D
        $bw.Write([uint32]0)            # miscFlag
        $bw.Write([uint32]1)            # arraySize -- the loader multiplies by this
        $bw.Write([uint32]0)            # miscFlags2

        $bw.Write($pixels)
    } finally { $bw.Dispose(); $fs.Dispose() }
}

$img = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $Gif))
try {
    $dim   = New-Object System.Drawing.Imaging.FrameDimension $img.FrameDimensionsList[0]
    $count = $img.GetFrameCount($dim)
    if ($count -lt 1) { throw "No frames in $Gif" }

    # Property 0x5100 (FrameDelay) is a packed array of 4-byte LE ints in 1/100 s.
    # A still GIF has no such property at all.
    $delays = @()
    try {
        $raw = $img.GetPropertyItem(0x5100).Value
        for ($i = 0; $i -lt $count; $i++) {
            $ms = [BitConverter]::ToInt32($raw, $i * 4) * 10
            $delays += $(if ($ms -le 0) { $DefaultDelay } else { $ms })
        }
    } catch {
        Write-Warning "No frame-delay data in the GIF; using ${DefaultDelay} ms for every frame."
        $delays = @($DefaultDelay) * $count
    }

    # Even sampling down to $MaxFrames, folding dropped delays into the kept frame.
    $keep = @(0..($count - 1))
    if ($count -gt $MaxFrames) {
        $keep = @(0..($MaxFrames - 1) | ForEach-Object { [int][Math]::Floor($_ * $count / $MaxFrames) })
        Write-Warning "GIF has $count frames; sampling down to $MaxFrames (the menu caps there). Total duration is preserved."
    }

    $kept = @()
    for ($k = 0; $k -lt $keep.Count; $k++) {
        $from = $keep[$k]
        $to   = if ($k + 1 -lt $keep.Count) { $keep[$k + 1] } else { $count }
        $sum  = 0
        for ($j = $from; $j -lt $to; $j++) { $sum += $delays[$j] }
        $kept += [pscustomobject]@{ Index = $from; Delay = $sum }
    }

    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    # Clear stale frames of either kind: a directory left over from the PNG-era
    # converter would otherwise keep feeding the loader files it cannot read.
    Get-ChildItem -LiteralPath $OutDir -ErrorAction SilentlyContinue |
        Where-Object { $_.Extension -in '.dds', '.png' } | Remove-Item -Force

    $manifest = @()
    for ($k = 0; $k -lt $kept.Count; $k++) {
        $img.SelectActiveFrame($dim, $kept[$k].Index) | Out-Null
        $name = '{0:d3}.dds' -f $k
        $bmp  = New-Object System.Drawing.Bitmap $img.Width, $img.Height
        try {
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            try { $g.DrawImage($img, 0, 0, $img.Width, $img.Height) } finally { $g.Dispose() }
            Write-Dds -Bitmap $bmp -Path (Join-Path $OutDir $name)
        } finally { $bmp.Dispose() }
        $manifest += [pscustomobject]@{ file = $name; delay = $kept[$k].Delay }
    }

    [pscustomobject]@{
        loop          = $Loop
        default_delay = $DefaultDelay
        frames        = $manifest
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $OutDir 'frames.json') -Encoding utf8

    $total = ($manifest | Measure-Object -Property delay -Sum).Sum
    Write-Host "$($manifest.Count) frame(s), $($img.Width)x$($img.Height), ${total} ms total -> $OutDir"
} finally {
    $img.Dispose()
}
