<#
.SYNOPSIS
  Initializes pinned submodules, generates VS2022 files, builds, tests, and launches LRenderDemo.
.NOTES
  Run from any location. All paths are derived from this script's repository location.
#>
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [switch]$SkipLaunch,
    [switch]$CommitVerifiedChanges,
    [ValidateNotNullOrEmpty()]
    [string]$CommitMessage = 'chore: commit verified changes'
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$logDirectory = Join-Path $projectRoot 'logs'
New-Item -ItemType Directory -Force -Path $logDirectory | Out-Null
$logFile = Join-Path $logDirectory ("bootstrap-{0}.log" -f (Get-Date -Format 'yyyy-MM-dd-HHmmss'))

function Write-Step {
    param([int]$Number, [int]$Total, [string]$Message)
    $line = "[{0}/{1}] {2}" -f $Number, $Total, $Message
    Write-Host $line
    Add-Content -LiteralPath $logFile -Value $line -Encoding UTF8
}

function Invoke-Checked {
    param([string]$Command, [string[]]$Arguments)
    $previousPreference = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        $commandOutput = & $Command @Arguments 2>&1
        $commandExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousPreference
    }
    if ($commandOutput) {
        $renderedOutput = ($commandOutput | Out-String).TrimEnd()
        Write-Host $renderedOutput
        Add-Content -LiteralPath $logFile -Value $renderedOutput -Encoding UTF8
    }
    if ($commandExitCode -ne 0) {
        throw "Command failed with exit code ${commandExitCode}: $Command $($Arguments -join ' ')"
    }
}

function Ensure-Submodule {
    param([string]$Name, [string]$Repository, [string]$Branch)
    $relativePath = "external/$Name"
    $registered = git config --file .gitmodules --get "submodule.$relativePath.path" 2>$null
    if ($LASTEXITCODE -ne 0 -or -not $registered) {
        if (Test-Path -LiteralPath (Join-Path $relativePath '.git')) {
            # Repair an interrupted add using Git commands so protected .git metadata stays managed by Git.
            Invoke-Checked git @('-C', $relativePath, 'fetch', '--depth', '1', 'origin', $Branch)
            Invoke-Checked git @('-C', $relativePath, 'checkout', '-B', $Branch, 'FETCH_HEAD')
            Invoke-Checked git @('config', '--file', '.gitmodules', "submodule.$relativePath.path", $relativePath)
            Invoke-Checked git @('config', '--file', '.gitmodules', "submodule.$relativePath.url", $Repository)
            Invoke-Checked git @('config', '--file', '.gitmodules', "submodule.$relativePath.branch", $Branch)
            Invoke-Checked git @('add', '.gitmodules', $relativePath)
        } else {
            Invoke-Checked git @(
                'submodule', 'add', '--force', '--depth', '1', '-b', $Branch, $Repository, $relativePath)
        }
    }
}

Push-Location $projectRoot
try {
    Write-Step 1 6 'Checking Git, CMake, and Visual Studio 2022...'
    Get-Command git -ErrorAction Stop | Out-Null
    Get-Command cmake -ErrorAction Stop | Out-Null
    $vsWhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
    if (-not (Test-Path -LiteralPath $vsWhere)) {
        throw 'Visual Studio Installer vswhere.exe was not found.'
    }
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) {
        throw 'Visual Studio 2022 with Desktop development with C++ is required.'
    }

    Write-Step 2 6 'Registering and downloading pinned Git submodules...'
    Ensure-Submodule 'DirectXTK' 'https://github.com/microsoft/DirectXTK.git' 'main'
    Ensure-Submodule 'imgui' 'https://github.com/ocornut/imgui.git' 'docking'
    Ensure-Submodule 'ImGuizmo' 'https://github.com/CedricGuillemet/ImGuizmo.git' 'master'
    Ensure-Submodule 'tinyobjloader' 'https://github.com/tinyobjloader/tinyobjloader.git' 'release'
    Invoke-Checked git @('submodule', 'update', '--init', '--recursive', '--depth', '1')

    Write-Step 3 6 'Generating the Visual Studio 2022 x64 solution...'
    Invoke-Checked cmake @('--preset', 'vs2022')

    Write-Step 4 6 "Building the $Configuration configuration..."
    Invoke-Checked cmake @('--build', '--preset', "vs2022-$($Configuration.ToLowerInvariant())")

    Write-Step 5 6 'Running CTest tests...'
    Invoke-Checked ctest @('--preset', "vs2022-$($Configuration.ToLowerInvariant())")

    Write-Step 6 6 'Bootstrap and verification completed.'
    $executable = Join-Path $projectRoot "build/vs2022/src/$Configuration/LRenderDemo.exe"
    if (-not $SkipLaunch) {
        if (-not (Test-Path -LiteralPath $executable)) {
            throw "Expected executable was not found: $executable"
        }
        $process = Start-Process -FilePath $executable -WorkingDirectory $projectRoot -PassThru
        Start-Sleep -Seconds 3
        if ($process.HasExited) {
            throw "LRenderDemo exited during startup with code $($process.ExitCode)."
        }
        Write-Host "LRenderDemo is running (PID $($process.Id))."
    }
    Write-Host "Visual Studio solution: build/vs2022/LRenderDemo.sln"
    Write-Host "Verification log: $logFile"
    if ($CommitVerifiedChanges) {
        Invoke-Checked git @('add', '--all')
        $pendingChanges = git status --porcelain
        if ($pendingChanges) {
            Invoke-Checked git @('commit', '-m', $CommitMessage)
        } else {
            Write-Host 'No verified changes to commit.'
        }
    }
} catch {
    $message = "[ERROR] $($_.Exception.Message)"
    Write-Error $message
    Add-Content -LiteralPath $logFile -Value $message -Encoding UTF8
    exit 1
} finally {
    Pop-Location
}
