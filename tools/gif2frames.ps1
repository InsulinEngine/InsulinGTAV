<#
.SYNOPSIS
    Converts an animated GIF into the frame directory the InsulinGTAV menu loads.

.DESCRIPTION
    Writes 000.png..NNN.png plus frames.json (per-frame delays in ms) into -OutDir.
    Copy that directory to /data/insulin/anim/<name>/ on the console.

    Uses .NET System.Drawing rather than ffmpeg or ImageMagick: neither is installed
    on the dev machine and neither ships with Windows, so depending on one would make
    this script fail on first use. Consequences, by design: PNG output only (DDS would
    need a block compressor) and frames keep the GIF's own resolution.

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
    Get-ChildItem -LiteralPath $OutDir -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force

    $manifest = @()
    for ($k = 0; $k -lt $kept.Count; $k++) {
        $img.SelectActiveFrame($dim, $kept[$k].Index) | Out-Null
        $name = '{0:d3}.png' -f $k
        $bmp  = New-Object System.Drawing.Bitmap $img.Width, $img.Height
        try {
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            try { $g.DrawImage($img, 0, 0, $img.Width, $img.Height) } finally { $g.Dispose() }
            $bmp.Save((Join-Path $OutDir $name), [System.Drawing.Imaging.ImageFormat]::Png)
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
