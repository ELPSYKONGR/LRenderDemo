<#
.SYNOPSIS
  Verifies downloaded learning assets against their SHA-256 manifests.
.DESCRIPTION
  Reads only repository-relative asset paths and fails with contextual diagnostics.
#>
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

$manifestSets = @(
    [pscustomobject]@{
        Name = 'classic test models'
        Root = Join-Path $projectRoot 'assets/test-scenes/downloads'
        Manifest = Join-Path $projectRoot 'assets/test-scenes/downloads/SHA256SUMS.txt'
    },
    [pscustomobject]@{
        Name = 'skybox'
        Root = Join-Path $projectRoot 'assets/skyboxes/downloads/directxtk-cubemap'
        Manifest = Join-Path $projectRoot 'assets/skyboxes/downloads/directxtk-cubemap/SHA256SUMS.txt'
    }
)

$requiredAssets = @(
    'assets/test-scenes/downloads/stanford-bunny/bunny.off',
    'assets/test-scenes/downloads/suzanne/Suzanne.gltf',
    'assets/test-scenes/downloads/suzanne/Suzanne.bin',
    'assets/test-scenes/downloads/damaged-helmet/DamagedHelmet.glb',
    'assets/test-scenes/downloads/metal-rough-spheres/MetalRoughSpheres.glb',
    'assets/test-scenes/downloads/sponza/Sponza.gltf',
    'assets/test-scenes/downloads/sponza/Sponza.bin',
    'assets/skyboxes/downloads/directxtk-cubemap/cubemap.dds'
)

foreach ($requiredAsset in $requiredAssets) {
    $requiredPath = Join-Path $projectRoot $requiredAsset
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Required learning asset is missing: $requiredAsset"
    }
}

$verifiedCount = 0
foreach ($manifestSet in $manifestSets) {
    if (-not (Test-Path -LiteralPath $manifestSet.Manifest -PathType Leaf)) {
        throw "Missing SHA-256 manifest for $($manifestSet.Name): $($manifestSet.Manifest)"
    }

    $assetRoot = [System.IO.Path]::GetFullPath($manifestSet.Root)
    $assetPrefix = $assetRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar) +
        [System.IO.Path]::DirectorySeparatorChar

    foreach ($line in Get-Content -LiteralPath $manifestSet.Manifest -Encoding UTF8) {
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }
        if ($line -notmatch '^([0-9a-fA-F]{64})\s+(.+)$') {
            throw "Invalid SHA-256 manifest line: $line"
        }

        $expectedHash = $Matches[1].ToLowerInvariant()
        $relativePath = $Matches[2]
        $assetPath = [System.IO.Path]::GetFullPath(
            (Join-Path $assetRoot ($relativePath -replace '/', '\')))
        if (-not $assetPath.StartsWith($assetPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "SHA-256 manifest path escapes its asset root: $relativePath"
        }
        if (-not (Test-Path -LiteralPath $assetPath -PathType Leaf)) {
            throw "SHA-256 manifest file is missing: $relativePath"
        }

        $actualHash = (Get-FileHash -LiteralPath $assetPath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne $expectedHash) {
            throw "SHA-256 mismatch: $relativePath; expected $expectedHash, actual $actualHash"
        }
        $verifiedCount++
    }
}

Write-Host "Learning asset verification passed: $verifiedCount files."
