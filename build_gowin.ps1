<#
build_gowin.ps1

PowerShell helper to run a Gowin project non-interactively.

Usage examples:
  # Quick build (synth -> pack -> bitgen) using PATH or common install locations
  .\build_gowin.ps1

  # Specify Gowin install root if tools are not on PATH
  .\build_gowin.ps1 -GowinRoot 'C:\Gowin\Gowin_pack'

  # Open project in associated GUI
  .\build_gowin.ps1 -OpenGui

  # Run only synthesis
  .\build_gowin.ps1 -Synthesize

Notes:
 - Adjust tool names in Find-Tool() if your Gowin installation uses different executable names.
 - This script doesn't program hardware by default. Use -Program to attempt programming (requires gowin_prog.exe).
#>

[CmdletBinding()]
param(
    [string]$GowinRoot = '',
    [string]$Project = (Join-Path $PSScriptRoot '1_low_mem_blink.gprj'),
    [switch]$OpenGui,
    [switch]$Synthesize,
    [switch]$PlaceRoute,
    [switch]$Bitgen,
    [switch]$Program
)

function Get-ToolByKeywords([string[]]$keywords) {
    # Search PATH first for any executable that contains one of the keywords
    foreach ($kw in $keywords) {
        $cmds = Get-Command *$kw* -ErrorAction SilentlyContinue | Where-Object {$_.CommandType -eq 'Application'}
        if ($cmds) { return $cmds[0].Path }
    }

    # Search GowinRoot recursively for matching exe names
    if ($GowinRoot) {
        foreach ($kw in $keywords) {
            try {
                $found = Get-ChildItem -Path $GowinRoot -Recurse -Filter "*$kw*.exe" -ErrorAction SilentlyContinue | Where-Object {!$_.PSIsContainer} | Select-Object -First 1
                if ($found) { return $found.FullName }
            } catch {}
        }
    }

    # Search common install candidates
    $candidates = @(
        'C:\Gowin\Gowin_pack\bin',
        'C:\Gowin\Gowin_V1.9.11.03_Education_x64\IDE\bin',
        'C:\Program Files\Gowin\Gowin_pack\bin',
        'C:\Program Files (x86)\Gowin\Gowin_pack\bin'
    )
    foreach ($c in $candidates) {
        foreach ($kw in $keywords) {
            try {
                $found = Get-ChildItem -Path $c -Recurse -Filter "*$kw*.exe" -ErrorAction SilentlyContinue | Where-Object {!$_.PSIsContainer} | Select-Object -First 1
                if ($found) { return $found.FullName }
            } catch {}
        }
    }

    return $null
}

param(
    [switch]$ListTools
)

function Find-Tool([string]$name) {
    # Try Get-Command first (PATH)
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Path }

    # Try provided GowinRoot (check several likely subfolders)
    if ($GowinRoot) {
        $subdirs = @('bin', 'IDE\bin', 'Gowin_pack\bin', 'bin\win64')
        foreach ($s in $subdirs) {
            $try = Join-Path $GowinRoot $s
            $full = Join-Path $try $name
            if (Test-Path $full) { return $full }
        }

        # Fallback: search recursively for likely executables when exact name differs
        try {
            $pattern = '*'+$name+'*'
            # If name contains keywords, narrow the pattern to common variants
            if ($name -match 'synth') { $pattern = '*synth*.exe' }
            elseif ($name -match 'pack') { $pattern = '*pack*.exe' }
            elseif ($name -match 'bitgen') { $pattern = '*bitgen*.exe' }
            elseif ($name -match 'prog|program') { $pattern = '*prog*.exe' }

            $found = Get-ChildItem -Path $GowinRoot -Filter $pattern -Recurse -ErrorAction SilentlyContinue | Where-Object {!$_.PSIsContainer} | Select-Object -First 1
            if ($found) { return $found.FullName }
        } catch {
            # ignore search errors (permissions, etc.)
        }
    }

    # Common install candidates
    $candidates = @(
        'C:\Gowin\Gowin_pack\bin',
        'C:\Gowin\Gowin_V1.9.11.03_Education_x64\IDE\bin',
        'C:\Program Files\Gowin\Gowin_pack\bin',
        'C:\Program Files (x86)\Gowin\Gowin_pack\bin'
    )
    foreach ($c in $candidates) {
        $full = Join-Path $c $name
        if (Test-Path $full) { return $full }
        # Try recursive search in candidate dir for likely exe names
        try {
            $pattern = '*'+$name+'*'
            if ($name -match 'synth') { $pattern = '*synth*.exe' }
            elseif ($name -match 'pack') { $pattern = '*pack*.exe' }
            elseif ($name -match 'bitgen') { $pattern = '*bitgen*.exe' }
            elseif ($name -match 'prog|program') { $pattern = '*prog*.exe' }
            $found = Get-ChildItem -Path $c -Filter $pattern -Recurse -ErrorAction SilentlyContinue | Where-Object {!$_.PSIsContainer} | Select-Object -First 1
            if ($found) { return $found.FullName }
        } catch {
        }
    }

    return $null
}

function Run-Tool($path, $args) {
    if (-not $path) {
        Write-Error "Tool not found."
        return 1
    }
    Write-Host "Running:`n  $path $args`n"
    $proc = Start-Process -FilePath $path -ArgumentList $args -NoNewWindow -Wait -PassThru
    return $proc.ExitCode
}

if ($OpenGui) {
    Write-Host "Opening project in associated application: $Project"
    Start-Process -FilePath $Project
    exit 0
}

if ($ListTools) {
    Write-Host "Detecting Gowin tools (using GowinRoot: $GowinRoot)"
    $synthTool = Get-ToolByKeywords @('synth','Synthesis')
    $packTool  = Get-ToolByKeywords @('modgen','pack','mod','gen')
    $bitgenTool= Get-ToolByKeywords @('bitgen','bit','modgen')
    $progTool  = Get-ToolByKeywords @('prog','program','programmer','gw_ctrl')
    $gwide     = Get-ToolByKeywords @('gw_ide','gw_ide.exe','gw_ide')
    Write-Host "Synthesis:    $synthTool"
    Write-Host "Pack/Place:   $packTool"
    Write-Host "Bitgen:       $bitgenTool"
    Write-Host "Programmer:   $progTool"
    Write-Host "IDE (GUI):    $gwide"
    exit 0
}

# Default behavior: do synth, pack, bitgen
if (-not ($Synthesize -or $PlaceRoute -or $Bitgen -or $Program)) {
    $Synthesize = $true
    $PlaceRoute = $true
    $Bitgen = $true
}

Write-Host "Using project: $Project"

if ($Synthesize) {
    # Try keyword-based discovery (GowinSynthesis.exe or variants)
    $synth = Get-ToolByKeywords @('synth','Synthesis')
    if (-not $synth) { Write-Error "Synthesis tool not found. Please set -GowinRoot or add tools to PATH."; exit 2 }
    $rc = Run-Tool $synth "-project `"$Project`""
    if ($rc -ne 0) { Write-Error "Synthesis failed (exit $rc)"; exit $rc }
}

if ($PlaceRoute) {
    # Try mod/gen style tool (GowinModGen.exe or similar)
    $pack = Get-ToolByKeywords @('modgen','pack','place','route')
    if (-not $pack) { Write-Error "Place & Route tool not found. Please set -GowinRoot or add tools to PATH."; exit 3 }
    $rc = Run-Tool $pack "-project `"$Project`""
    if ($rc -ne 0) { Write-Error "Place & Route failed (exit $rc)"; exit $rc }
}

if ($Bitgen) {
    # Try bitgen or module generator tools
    $bitgen = Get-ToolByKeywords @('bitgen','modgen','gen')
    if (-not $bitgen) { Write-Error "Bitstream generator not found. Please set -GowinRoot or add tools to PATH."; exit 4 }
    $rc = Run-Tool $bitgen "-project `"$Project`""
    if ($rc -ne 0) { Write-Error "Bitstream generation failed (exit $rc)"; exit $rc }
}

if ($Program) {
    $prog = Get-ToolByKeywords @('prog','program','programmer','gw_ctrl')
    if (-not $prog) { Write-Error "Programmer not found. Please set -GowinRoot or add tools to PATH."; exit 5 }
    $rc = Run-Tool $prog "-project `"$Project`""
    if ($rc -ne 0) { Write-Error "Programming failed (exit $rc)"; exit $rc }
}

Write-Host "Gowin build finished successfully."
exit 0
