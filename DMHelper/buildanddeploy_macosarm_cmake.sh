#!/usr/bin/env bash
set -e

SKIP_BUILD=0
if [[ "$1" == "--skip-build" ]]; then
    SKIP_BUILD=1
fi

SCRIPT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_ROOT"

QT_VERSION="6.10.3"

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
BUILD_DIR="$SCRIPT_ROOT/build-macosarm-release"
BIN_DIR="$SCRIPT_ROOT/bin-macosarm"

QT_BIN_DIR="$QT_DIR/bin"
MACDEPLOYQT="$QT_BIN_DIR/macdeployqt"
QT6_CMAKE_DIR="$QT_DIR/lib/cmake/Qt6"

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

read -p "Completely rebuild and redeploy DMHelper (macOS)? (y/n) " CONFIRM
if [[ "$CONFIRM" != "y" ]]; then
    echo "Aborted."
    exit 0
fi

section "Checking build tools"
assert_exists "$(xcode-select -p)" "Xcode Command Line Tools"
assert_exists "$QT_DIR" "Qt directory"
assert_exists "$QT6_CMAKE_DIR" "Qt6 CMake config"
assert_exists "$MACDEPLOYQT" "macdeployqt"

export CMAKE_PREFIX_PATH="$QT_DIR"
export Qt6_DIR="$QT6_CMAKE_DIR"

section "Preparing output directories"
rm -rf "$BIN_DIR"
mkdir -p \
    "$BIN_DIR/DMHelper.app/Contents" \
    "$BIN_DIR/DMHelper.app/Contents/Frameworks" \
    "$BIN_DIR/DMHelper.app/Contents/Frameworks/pkgconfig" \
    "$BIN_DIR/DMHelper.app/Contents/Frameworks/plugins" \
    "$BIN_DIR/DMHelper.app/Contents/Resources"

if [[ "$SKIP_BUILD" -eq 0 ]]; then
    section "Configuring and building DMHelper (macOS)"
    rm -rf "$BUILD_DIR"
    mkdir "$BUILD_DIR"
    cmake \
        -S "$SRC_DIR" \
        -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DDMH_MACOS_ARCH=arm64 \
        -G "Xcode"
    cmake --build "$BUILD_DIR" --config Release
else
    section "Skipping build (using existing binaries)"
fi

section "Copying build artifacts"
APP_PATH="$BUILD_DIR/Release/DMHelper.app"
assert_exists "$APP_PATH" "DMHelper.app"
cp -R "$APP_PATH" "$BIN_DIR/"
cp -R "$SRC_DIR/bin-macosarm/Info.plist" "$BIN_DIR/DMHelper.app/Contents/"
cp -R "$SRC_DIR/bin-macosarm/pkgconfig/"* "$BIN_DIR/DMHelper.app/Contents/Frameworks/pkgconfig"
cp -R "$SRC_DIR/bin-macosarm/vlc/plugins/"* "$BIN_DIR/DMHelper.app/Contents/Frameworks/plugins"
cp -R "$SRC_DIR/bin-macosarm/"libvlc*.dylib "$BIN_DIR/DMHelper.app/Contents/Frameworks/"

section "Running macdeployqt"
"$MACDEPLOYQT" "$BIN_DIR/DMHelper.app" -always-overwrite -verbose=1

section "Copying resources into app bundle"
RESOURCES_DIR="$BIN_DIR/DMHelper.app/Contents/Resources"
cp -R "$SRC_DIR/bestiary/"* "$RESOURCES_DIR/"
cp -R "$SRC_DIR/resources/"* "$RESOURCES_DIR/"
cp -R "$SRC_DIR/doc/"* "$RESOURCES_DIR/"
cp "$SRC_DIR/bin-macosarm/DMHelper.icns" "$RESOURCES_DIR/"

section "Creating ZIP distribution"
cd "$BIN_DIR"
zip -r "$SCRIPT_ROOT/DMHelper-macOS-arm64-release.zip" DMHelper.app

echo ""
echo "Build completed successfully (macOS arm64)"