<#
.SYNOPSIS
  Downloads the pinned cubemap DDS and its upstream license.
.DESCRIPTION
  Paths are derived from the repository. Existing files are kept unless -Force is used.
#>
[CmdletBinding()]
param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$assetDirectory = Join-Path $projectRoot 'assets/skyboxes/downloads/directxtk-cubemap'
$sourceCommit = 'fabb928cf620381dfc188d52040a5a8e32bd1aec'
$sourceRoot = "https://raw.githubusercontent.com/walbourn/directxtktest/$sourceCommit"

$downloadItems = @(
    [pscustomobject]@{
        FileName = 'cubemap.dds'
        Url = "$sourceRoot/EffectsTest/cubemap.dds"
    },
    [pscustomobject]@{
        FileName = 'SOURCE_LICENSE.txt'
        Url = "$sourceRoot/LICENSE"
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

New-Item -ItemType Directory -Force -Path $assetDirectory | Out-Null
foreach ($item in $downloadItems) {
    Invoke-AssetDownload -Url $item.Url -Destination (Join-Path $assetDirectory $item.FileName)
}

$hashLines = Get-ChildItem -LiteralPath $assetDirectory -File |
    Where-Object { $_.Name -ne 'SHA256SUMS.txt' } |
    Sort-Object Name |
    ForEach-Object {
        $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToLowerInvariant()
        "$hash  $($_.Name)"
    }
$hashPath = Join-Path $assetDirectory 'SHA256SUMS.txt'
$hashLines | Set-Content -LiteralPath $hashPath -Encoding UTF8

Write-Host "Completed. Cubemap directory: $assetDirectory"
Write-Host "Checksum manifest: $hashPath"
