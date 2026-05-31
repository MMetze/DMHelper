param (
    [switch]$SkipBuild
)

# =========================
# Bootstrap / Root
# =========================

$ErrorActionPreference = "Stop"
$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $ScriptRoot

# =========================
# Configuration
# =========================

# ------------------------
# Qt configuration
# Cross-compiled ARM64: Qt tools (windeployqt, binarycreator) are x64 host
# executables even though the Qt libraries themselves target ARM64.
# ------------------------

if ($env:QT_ROOT_DIR) {
    $QtDir = $env:QT_ROOT_DIR
    $QtRoot = Split-Path (Split-Path $QtDir -Parent) -Parent
    $QtVersion          = "6.10.3"
    $QtInstallerVersion = "4.7"
    $MsvcYear           = "2022"
    Write-Host "Using Qt from environment: $QtDir"
} else {
    $QtRoot             = "C:\Qt"
    $QtVersion          = "6.10.3"
    $QtInstallerVersion = "4.10"
    $MsvcYear           = "2022"
    $QtDir              = Join-Path $QtRoot "$QtVersion\msvc${MsvcYear}_arm64"
    Write-Host "Using default Qt path: $QtDir"
}

$SevenZip = "C:\Program Files\7-Zip\7z.exe"
if (-not (Test-Path $SevenZip)) {
    $SevenZip = "7z.exe"
}

$SrcDir   = Join-Path $ScriptRoot "src"
$BuildDir = Join-Path $ScriptRoot "build-arm64-release"
$BinDir   = Join-Path $ScriptRoot "binarm64"

Write-Host ""
Write-Host "================================================================================"
Write-Host "Execution Directory: $ScriptRoot"
Write-Host "Source Directory:    $SrcDir"
Write-Host "Build Directory:     $BuildDir"
Write-Host "Binary Directory:    $BinDir"
Write-Host "================================================================================"
Write-Host ""

# =========================
# Helper functions
# =========================

function Write-Section($msg) {
    Write-Host ""
    Write-Host "================================================================================"
    Write-Host $msg
    Write-Host "================================================================================"
}

function Assert-Exists($path, $name) {
    if (-not (Test-Path $path)) {
        throw "$name not found: $path"
    }
}

# =========================
# Confirmation
# =========================

$choice = Read-Host "Completely rebuild and redeploy DMHelper (ARM64)? (y/n)"
if ($choice -ne "y") {
    Write-Host "Aborted."
    exit 0
}

# =========================
# Locate Visual Studio
# =========================

Write-Section "Locating Visual Studio"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
Assert-Exists $vswhere "vswhere"

$VsInstallDir = & $vswhere `
    -latest `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $VsInstallDir) {
    throw "Visual Studio with C++ tools not found"
}

Write-Host "Found Visual Studio at $VsInstallDir"

# =========================
# Initialize MSVC environment (cross-compile: host=x64, target=arm64)
# =========================

Write-Section "Initializing MSVC environment (cross: x64 host -> arm64 target)"

$VsDevCmd = Join-Path $VsInstallDir "Common7\Tools\VsDevCmd.bat"
Assert-Exists $VsDevCmd "VsDevCmd.bat"

cmd /c "`"$VsDevCmd`" -arch=arm64 -host_arch=x64 && set" |
    ForEach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw "MSVC compiler not available"
}

# =========================
# Qt configuration
# windeployqt and binarycreator are x64 host tools shipped alongside the
# ARM64 cross-compiled Qt package.
# =========================

$Qt6Dir          = Join-Path $QtDir "lib\cmake\Qt6"
$QtBinDir        = Join-Path $QtDir "bin"
$WinDeployQt     = Join-Path $QtBinDir "windeployqt.exe"
$QtIfwDir        = Join-Path $QtRoot "Tools\QtInstallerFramework\$QtInstallerVersion\bin"
$BinaryCreator   = Join-Path $QtIfwDir "binarycreator.exe"

Assert-Exists $QtDir        "Qt ARM64 MSVC directory"
Assert-Exists $Qt6Dir       "Qt6 CMake config"
Assert-Exists $WinDeployQt  "windeployqt"
Assert-Exists $BinaryCreator "binarycreator"
Assert-Exists $SevenZip     "7-zip"

$env:CMAKE_PREFIX_PATH = $QtDir
$env:Qt6_DIR           = $Qt6Dir

# =========================
# Prepare output directories
# =========================

Write-Section "Preparing output directories"

Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $BinDir

New-Item -ItemType Directory -Force -Path @(
    $BinDir
    "$BinDir\config"
    "$BinDir\packages\com.dmhelper.app\meta"
    "$BinDir\packages\com.dmhelper.app\data"
    "$BinDir\packages\com.dmhelper.app\data\bestiary"
    "$BinDir\packages\com.dmhelper.app\data\doc"
    "$BinDir\packages\com.dmhelper.app\data\pkgconfig"
    "$BinDir\packages\com.dmhelper.app\data\plugins"
    "$BinDir\packages\com.dmhelper.app\data\resources\tables"
) | Out-Null

Copy-Item -Recurse -Force "$SrcDir\installer\*" $BinDir

# Remove the x64 and x86 installscripts; rename the arm64 one to installscript.qs
Remove-Item -ErrorAction SilentlyContinue "$BinDir\packages\com.dmhelper.app\meta\installscript64.qs"
Remove-Item -ErrorAction SilentlyContinue "$BinDir\packages\com.dmhelper.app\meta\installscript32.qs"
Rename-Item `
    "$BinDir\packages\com.dmhelper.app\meta\installscriptarm64.qs" `
    "installscript.qs"

# =========================
# Build
# =========================

if (-not $SkipBuild) {
    Write-Section "Configuring and building DMHelper (ARM64)"

    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $BuildDir
    New-Item -ItemType Directory $BuildDir | Out-Null

    cmake -S $SrcDir -B $BuildDir -G Ninja `
        -DCMAKE_BUILD_TYPE=Release `
        -DCMAKE_PREFIX_PATH=$QtDir `
        -DQt6_DIR=$Qt6Dir `
        -DDMH_WINDOWS_ARCH=arm64
    cmake --build $BuildDir --config Release
}
else {
    Write-Section "Skipping build (using existing binaries)"
}

# =========================
# Copy build artifacts
# =========================

Write-Section "Copying build artifacts"

Copy-Item `
    "$BuildDir\DMHelper.exe" `
    "$BinDir\packages\com.dmhelper.app\data\"

Copy-Item `
    "$QtDir\bin\Qt6Xml.dll" `
    "$BinDir\packages\com.dmhelper.app\data\"

Copy-Item -Recurse -Force `
    "$SrcDir\bin-winarm64\*" `
    "$BinDir\packages\com.dmhelper.app\data\"

Copy-Item -Recurse -Force "$SrcDir\bestiary\*"  "$BinDir\packages\com.dmhelper.app\data\resources\"
Copy-Item -Recurse -Force "$SrcDir\doc\*"       "$BinDir\packages\com.dmhelper.app\data\doc\"
Copy-Item -Recurse -Force "$SrcDir\resources\*" "$BinDir\packages\com.dmhelper.app\data\resources\"

# =========================
# Qt deployment
# =========================

Write-Section "Running windeployqt"

& $WinDeployQt `
    --compiler-runtime `
    --no-opengl-sw `
    --no-svg `
    "$BinDir\packages\com.dmhelper.app\data\DMHelper.exe"

# =========================
# Create installer
# =========================

Write-Section "Creating installer"

Push-Location $BinDir
& $BinaryCreator `
    -c config\config_winarm64.xml `
    -p packages `
    "DMHelper-arm64-release-Installer"
Pop-Location

Move-Item `
    "$BinDir\DMHelper-arm64-release-Installer.exe" `
    "$ScriptRoot\DMHelper-arm64-release-Installer.exe" `
    -Force

# =========================
# Create ZIP
# =========================

Write-Section "Creating ZIP distribution"

& $SevenZip `
    a -tzip archive.zip `
    "$BinDir\packages\com.dmhelper.app\data\*"

Move-Item archive.zip "DMHelper-arm64-release.zip" -Force

Write-Host ""
Write-Host "Build completed successfully."
