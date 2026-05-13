#!/usr/bin/env bash
set -e

# =========================
# Arguments
# =========================

SKIP_BUILD=0
if [[ "$1" == "--skip-build" ]]; then
    SKIP_BUILD=1
fi

# =========================
# Bootstrap / Root
# =========================

SCRIPT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_ROOT"

# =========================
# Configuration
# =========================

QT_VERSION="6.10.1"
QT_IFW_VERSION="4.7"

if [[ -n "$QT_ROOT_DIR" ]]; then
    QT_DIR="$QT_ROOT_DIR"
    QT_ROOT="$(cd "$QT_DIR/../.." && pwd)"
    echo "Using Qt from environment: $QT_DIR"
else
    QT_ROOT="$HOME/Qt"
    QT_DIR="$QT_ROOT/$QT_VERSION/macos"
    echo "Using default Qt path: $QT_DIR"
fi

SRC_DIR="$SCRIPT_ROOT/src"
BUILD_DIR="$SCRIPT_ROOT/build-macos-release"
BIN_DIR="$SCRIPT_ROOT/bin-macos"

QT_BIN_DIR="$QT_DIR/bin"
MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"
QT6_CMAKE_DIR="$QT_DIR/lib/cmake/Qt6"

QT_IFW_DIR="$QT_ROOT/Tools/QtInstallerFramework/$QT_IFW_VERSION/bin"
BINARY_CREATOR="$QT_IFW_DIR/binarycreator"

# =========================
# Helpers
# =========================

section() {
    echo ""
    echo "================================================================================"
    echo "$1"
    echo "================================================================================"
}

assert_exists() {
    if [[ ! -e "$1" ]]; then
        echo "ERROR: $2 not found: $1"
        exit 1
    fi
}

# =========================
# Confirmation
# =========================

read -p "Completely rebuild and redeploy DMHelper (macOS)? (y/n) " CONFIRM
if [[ "$CONFIRM" != "y" ]]; then
    echo "Aborted."
    exit 0
fi

# =========================
# Tool checks
# =========================

section "Checking build tools"

assert_exists "$(xcode-select -p)" "Xcode Command Line Tools"
assert_exists "$QT_DIR" "Qt directory"
assert_exists "$QT6_CMAKE_DIR" "Qt6 CMake config"
assert_exists "$MACDEPLOYQT" "macdeployqt"
assert_exists "$BINARY_CREATOR" "binarycreator"

export CMAKE_PREFIX_PATH="$QT_DIR"
export Qt6_DIR="$QT6_CMAKE_DIR"

# =========================
# Prepare output directories
# =========================

section "Preparing output directories"

rm -rf "$BIN_DIR"

mkdir -p \
    "$BIN_DIR/config" \
    "$BIN_DIR/packages/com.dmhelper.app/meta" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/pkgconfig" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/plugins" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Resources"

cp -R "$SRC_DIR/installer/"* "$BIN_DIR"

mv \
    "$BIN_DIR/packages/com.dmhelper.app/meta/installscript64.qs" \
    "$BIN_DIR/packages/com.dmhelper.app/meta/installscript.qs"

# =========================
# Build
# =========================

if [[ "$SKIP_BUILD" -eq 0 ]]; then
    section "Configuring and building DMHelper (macOS)"

    rm -rf "$BUILD_DIR"
    mkdir "$BUILD_DIR"

    cmake \
        -S "$SRC_DIR" \
        -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -G "Xcode"

    cmake --build "$BUILD_DIR" --config Release
else
    section "Skipping build (using existing binaries)"
fi

# =========================
# Copy build artifacts
# =========================

section "Copying build artifacts"

APP_PATH="$BUILD_DIR/Release/DMHelper.app"
assert_exists "$APP_PATH" "DMHelper.app"

cp -R "$APP_PATH" \
    "$BIN_DIR/packages/com.dmhelper.app/data/"

cp -R "$SRC_DIR/bin-macos/Info.plist" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/"

cp -R "$SRC_DIR/bin-macos/pkgconfig/"* \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/pkgconfig"

cp -R "$SRC_DIR/bin-macos/vlc/plugins/"* \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/plugins"

# Copy VLC dylibs into the app bundle's Frameworks directory
cp -R "$SRC_DIR/bin-macos/"libvlc*.dylib \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/"

# =========================
# Qt deployment
# =========================

section "Running macdeployqt"

"$MACDEPLOYQT" \
    "$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app" \
    -always-overwrite \
    -verbose=1

# =========================
# Copy resources into the app bundle
# (must be after macdeployqt so they are not overwritten)
# =========================

section "Copying resources into app bundle"

RESOURCES_DIR="$BIN_DIR/packages/com.dmhelper.app/data/DMHelper.app/Contents/Resources"

cp -R "$SRC_DIR/bestiary/"*  "$RESOURCES_DIR/"
cp -R "$SRC_DIR/resources/"* "$RESOURCES_DIR/"
cp -R "$SRC_DIR/doc/"*       "$RESOURCES_DIR/"
cp "$SRC_DIR/bin-macos/DMHelper.icns" "$RESOURCES_DIR/"

# =========================
# Create installer (.app or .dmg)
# =========================

section "Creating installer (Qt IFW)"

pushd "$BIN_DIR" > /dev/null

"$BINARY_CREATOR" \
    -c config/config_mac.xml \
    -p packages \
    DMHelper-macOS-Installer

popd > /dev/null

mv \
    "$BIN_DIR/DMHelper-macOS-Installer.app" \
    "$SCRIPT_ROOT/DMHelper-macOS-Installer.app"

# =========================
# Create ZIP
# =========================

section "Creating ZIP distribution"

cd "$BIN_DIR/packages/com.dmhelper.app/data"
zip -r "$SCRIPT_ROOT/DMHelper-macOS-release.zip" .

# =========================
# Done
# =========================

echo ""
echo "Build completed successfully (macOS)"
