<#
    Tests tools/gif2frames.ps1 against a hand-built fixture GIF.
    Run:  pwsh -File tests/gif2frames_test.ps1
    Exit: 0 all passed, 1 something failed.
#>
$ErrorActionPreference = 'Stop'
$root    = Split-Path -Parent $PSScriptRoot
$script  = Join-Path $root 'tools\gif2frames.ps1'
$work    = Join-Path $root 'build\fixture'
$failed  = 0

function Check([string]$what, $got, $want) {
    if ("$got" -ne "$want") { Write-Host "FAIL $what : got '$got', want '$want'"; $script:failed++ }
    else                    { Write-Host "ok   $what = $got" }
}

# --- fixture: GIF89a, 1x1, two frames, delays 10 and 20 (1/100 s) ------------
$bytes = [byte[]]@(0x47,0x49,0x46,0x38,0x39,0x61, 0x01,0x00, 0x01,0x00, 0x80,0x00,0x00,
          0x00,0x00,0x00, 0xFF,0xFF,0xFF, 0x21,0xFF,0x0B) +
         [System.Text.Encoding]::ASCII.GetBytes("NETSCAPE2.0") +
         [byte[]]@(0x03,0x01,0x00,0x00,0x00,
          0x21,0xF9,0x04,0x00,0x0A,0x00,0x00,0x00,
          0x2C,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00, 0x02,0x02,0x44,0x01,0x00,
          0x21,0xF9,0x04,0x00,0x14,0x00,0x00,0x00,
          0x2C,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00, 0x02,0x02,0x44,0x01,0x00,
          0x3B)

New-Item -ItemType Directory -Force -Path $work | Out-Null
$gif = Join-Path $work 'two.gif'
[System.IO.File]::WriteAllBytes($gif, $bytes)

# The fixture must be readable, or every assertion below tests nothing.
Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile($gif)
try {
    $dim = New-Object System.Drawing.Imaging.FrameDimension $img.FrameDimensionsList[0]
    Check 'fixture frame count' $img.GetFrameCount($dim) 2
    Check 'fixture delay table' ($img.GetPropertyItem(0x5100).Value -join ',') '10,0,0,0,20,0,0,0'
} finally { $img.Dispose() }

# --- full conversion --------------------------------------------------------
$out = Join-Path $work 'out'
if (Test-Path $out) { Remove-Item $out -Recurse -Force }
& pwsh -File $script -Gif $gif -OutDir $out | Out-Null

Check 'wrote 000.png'  (Test-Path (Join-Path $out '000.png')) 'True'
Check 'wrote 001.png'  (Test-Path (Join-Path $out '001.png')) 'True'
Check 'wrote manifest' (Test-Path (Join-Path $out 'frames.json')) 'True'

$m = Get-Content (Join-Path $out 'frames.json') -Raw | ConvertFrom-Json
Check 'manifest frame count' $m.frames.Count 2
Check 'frame 0 file'         $m.frames[0].file '000.png'
Check 'frame 0 delay (ms)'   $m.frames[0].delay 100
Check 'frame 1 delay (ms)'   $m.frames[1].delay 200
Check 'loop flag'            $m.loop 'True'
Check 'default delay'        $m.default_delay 66

# --- down-sampling keeps total duration and says so -------------------------
$out1 = Join-Path $work 'out1'
if (Test-Path $out1) { Remove-Item $out1 -Recurse -Force }
& pwsh -File $script -Gif $gif -OutDir $out1 -MaxFrames 1 3>$null | Out-Null

$m1 = Get-Content (Join-Path $out1 'frames.json') -Raw | ConvertFrom-Json
Check 'sampled frame count'   $m1.frames.Count 1
Check 'folded delay (100+200)' $m1.frames[0].delay 300

if ($failed) { Write-Host "`n$failed FAILED"; exit 1 }
Write-Host "`nall passed"; exit 0
