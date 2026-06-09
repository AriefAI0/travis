# Runs the Qt app from the CMake build folder with local Qt and GStreamer runtime paths.

param(
    [string]$BuildDir = "build/mingw-debug",
    [string]$GStreamerRuntimeRoot = "runtime/gstreamer/1.0/msvc_x86_64",
    [string]$QtRoot = "",
    [switch]$NoBuild,
    [switch]$NoLaunch
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
    param([string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    $repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
    return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

function Assert-ExistingPath {
    param(
        [string]$Path,
        [string]$Message
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "$Message Path: $Path"
    }
}

function Read-CMakeCacheValue {
    param(
        [string]$CachePath,
        [string]$Name
    )

    $line = Get-Content -LiteralPath $CachePath |
        Where-Object { $_ -match "^$([regex]::Escape($Name)):[^=]+=" } |
        Select-Object -First 1

    if (-not $line) {
        return ""
    }

    return ($line -split "=", 2)[1]
}

$buildRoot = Resolve-RepoPath $BuildDir
$runtimeRoot = Resolve-RepoPath $GStreamerRuntimeRoot
$cachePath = Join-Path $buildRoot "CMakeCache.txt"
$appPath = Join-Path $buildRoot "travis_app.exe"

Assert-ExistingPath $buildRoot "CMake build folder was not found."
Assert-ExistingPath $cachePath "CMake cache was not found. Configure the build before running the app."
Assert-ExistingPath $runtimeRoot "Bundled GStreamer runtime folder was not found."

if ([string]::IsNullOrWhiteSpace($QtRoot)) {
    $QtRoot = Read-CMakeCacheValue $cachePath "CMAKE_PREFIX_PATH"
}

if ([string]::IsNullOrWhiteSpace($QtRoot)) {
    $qtDir = Read-CMakeCacheValue $cachePath "Qt6_DIR"
    if (-not [string]::IsNullOrWhiteSpace($qtDir)) {
        $QtRoot = [System.IO.Path]::GetFullPath((Join-Path $qtDir "..\..\.."))
    }
}

if ([string]::IsNullOrWhiteSpace($QtRoot)) {
    throw "Qt root could not be inferred from CMakeCache.txt. Pass -QtRoot C:/Qt/<version>/mingw_64."
}

$qtRootPath = Resolve-RepoPath $QtRoot
$qtBinPath = Join-Path $qtRootPath "bin"
$gstBinPath = Join-Path $runtimeRoot "bin"
$gstPluginPath = Join-Path $runtimeRoot "lib/gstreamer-1.0"

Assert-ExistingPath $qtBinPath "Qt bin folder was not found."
Assert-ExistingPath $gstBinPath "GStreamer bin folder was not found."
Assert-ExistingPath $gstPluginPath "GStreamer plugin folder was not found."

if (-not $NoBuild) {
    cmake --build $buildRoot
}

Assert-ExistingPath $appPath "Qt app executable was not found after build."

# Keep runtime changes process-local so developer machines are not polluted.
$env:PATH = "$qtBinPath;$gstBinPath;$env:PATH"
$env:GST_PLUGIN_PATH_1_0 = $gstPluginPath
$env:GST_PLUGIN_SYSTEM_PATH_1_0 = $gstPluginPath
$env:TRAVIS_GSTREAMER_RUNTIME_ROOT = $runtimeRoot

Write-Host "Build: $buildRoot"
Write-Host "Qt: $qtRootPath"
Write-Host "GStreamer: $runtimeRoot"

if ($NoLaunch) {
    Write-Host "NoLaunch enabled. Runtime environment validated."
    exit 0
}

& $appPath
exit $LASTEXITCODE
