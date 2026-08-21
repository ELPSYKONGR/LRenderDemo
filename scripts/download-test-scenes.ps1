<#
.SYNOPSIS
  Downloads classic graphics test models and creates a SHA-256 manifest.
.DESCRIPTION
  Paths are derived from the repository. Existing files are kept unless -Force is used.
#>
[CmdletBinding()]
param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$downloadRoot = Join-Path $projectRoot 'assets/test-scenes/downloads'

$downloadItems = @(
    [pscustomobject]@{
        Directory = 'stanford-bunny'
        FileName = 'bunny.off'
        Url = 'https://raw.githubusercontent.com/libigl/libigl-tutorial-data/master/bunny.off'
    },
    [pscustomobject]@{
        Directory = 'stanford-bunny'
        FileName = 'MIRROR_README.md'
        Url = 'https://raw.githubusercontent.com/libigl/libigl-tutorial-data/master/README.md'
    },
    [pscustomobject]@{
        Directory = 'suzanne'
        FileName = 'SOURCE_README.md'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/Suzanne/README.md'
    },
    [pscustomobject]@{
        Directory = 'sponza'
        FileName = 'SOURCE_README.md'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/Sponza/README.md'
    },
    [pscustomobject]@{
        Directory = 'metal-rough-spheres'
        FileName = 'MetalRoughSpheres.glb'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/MetalRoughSpheres/glTF-Binary/MetalRoughSpheres.glb'
    },
    [pscustomobject]@{
        Directory = 'metal-rough-spheres'
        FileName = 'SOURCE_README.md'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/MetalRoughSpheres/README.md'
    },
    [pscustomobject]@{
        Directory = 'damaged-helmet'
        FileName = 'DamagedHelmet.glb'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/DamagedHelmet/glTF-Binary/DamagedHelmet.glb'
    },
    [pscustomobject]@{
        Directory = 'damaged-helmet'
        FileName = 'SOURCE_README.md'
        Url = 'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/Models/DamagedHelmet/README.md'
    }
)

function Invoke-AssetDownload {
    param(
        [Parameter(Mandatory)] [string]$Url,
        [Parameter(Mandatory)] [string]$Destination
    )

    if (-not $Force -and (Test-Path -LiteralPath $Destination) -and
        (Get-Item -LiteralPath $Destination).Length -gt 0) {
        Write-Host "[skip] Existing file: $Destination"
        return
    }

    $temporaryPath = "$Destination.download"
    Write-Host "[download] $Url"
    & curl.exe --fail --location --silent --show-error --retry 10 --retry-delay 2 `
        --retry-all-errors --connect-timeout 30 --speed-limit 1024 --speed-time 60 `
        --continue-at - --ssl-no-revoke `
        --user-agent 'LRenderDemo-Asset-Downloader/1.0' `
        --output $temporaryPath $Url
    if ($LASTEXITCODE -ne 0) {
        throw "curl.exe failed with exit code ${LASTEXITCODE}: $Url"
    }
    if ((Get-Item -LiteralPath $temporaryPath).Length -eq 0) {
        throw "Downloaded file is empty: $Url"
    }
    Move-Item -LiteralPath $temporaryPath -Destination $Destination -Force
}

function Sync-KhronosGltfDirectory {
    param(
        [Parameter(Mandatory)] [string]$ModelName,
        [Parameter(Mandatory)] [string]$DestinationDirectory,
        [Parameter(Mandatory)] [object]$RepositoryTree
    )

    $sourcePrefix = "Models/$ModelName/glTF/"
    $sourceFiles = @($RepositoryTree.tree | Where-Object {
        $_.type -eq 'blob' -and $_.path.StartsWith($sourcePrefix)
    })
    if ($sourceFiles.Count -eq 0) {
        throw "No glTF files found in Khronos repository tree for $ModelName."
    }

    foreach ($sourceFile in $sourceFiles) {
        $relativePath = $sourceFile.path.Substring($sourcePrefix.Length)
        $destination = Join-Path $DestinationDirectory $relativePath
        $parentDirectory = Split-Path -Parent $destination
        New-Item -ItemType Directory -Force -Path $parentDirectory | Out-Null
        $url = "https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Assets/main/$($sourceFile.path)"
        Invoke-AssetDownload -Url $url -Destination $destination
    }
}

New-Item -ItemType Directory -Force -Path $downloadRoot | Out-Null

foreach ($item in $downloadItems) {
    $assetDirectory = Join-Path $downloadRoot $item.Directory
    New-Item -ItemType Directory -Force -Path $assetDirectory | Out-Null
    Invoke-AssetDownload -Url $item.Url -Destination (Join-Path $assetDirectory $item.FileName)
}

$treePath = Join-Path $downloadRoot '.khronos-tree.json'
try {
    Invoke-AssetDownload `
        -Url 'https://api.github.com/repos/KhronosGroup/glTF-Sample-Assets/git/trees/main?recursive=1' `
        -Destination $treePath
    $repositoryTree = Get-Content -Raw -Encoding UTF8 -LiteralPath $treePath | ConvertFrom-Json
    Sync-KhronosGltfDirectory `
        -ModelName 'Suzanne' `
        -DestinationDirectory (Join-Path $downloadRoot 'suzanne') `
        -RepositoryTree $repositoryTree
    Sync-KhronosGltfDirectory `
        -ModelName 'Sponza' `
        -DestinationDirectory (Join-Path $downloadRoot 'sponza') `
        -RepositoryTree $repositoryTree
} finally {
    if (Test-Path -LiteralPath $treePath) {
        Remove-Item -LiteralPath $treePath -Force
    }
}

$hashLines = Get-ChildItem -LiteralPath $downloadRoot -File -Recurse |
    Where-Object { $_.Name -notin @('SHA256SUMS.txt', '.gitkeep') } |
    Sort-Object FullName |
    ForEach-Object {
        $relativePath = $_.FullName.Substring($downloadRoot.Length + 1).Replace('\', '/')
        $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
        "$hash  $relativePath"
    }

$hashFile = Join-Path $downloadRoot 'SHA256SUMS.txt'
$hashLines | Set-Content -LiteralPath $hashFile -Encoding UTF8
Write-Host "Completed. Model directory: $downloadRoot"
Write-Host "Checksum manifest: $hashFile"
