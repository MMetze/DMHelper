#!/usr/bin/env sh

set -e

die() {
  echo $1
  exit 1
}

message() {
  echo ................................................................................
  echo $1
  echo ................................................................................
}

clean_old_artifacts() {
  rm -rf {build-clang_64bit-Release,bin64,"DMHelper macOS release.zip","DMHelper macOS release Installer.app"}
}


setup_build() {
  mkdir -p {build-clang_64bit-Release,bin64/config,bin64/packages/com.dmhelper.app/{meta,data}}
  cp -R src/installer/ bin64/
  mv bin64/packages/com.dmhelper.app/meta/installscript64.qs bin64/packages/com.dmhelper.app/meta/installscript.qs
}

message "Building DMHelper for MacOS"

# capture current directory.
message "Running $SCRIPT_DIR/buildanddeploymac.sh from $CUR_DIR"

# only define QT_DIR if it's not already defined in the environment.
QT_VERSION=5.15.2
QT_DIR=${QT_DIR:-"$HOME/Qt"}
QT_BIN=${QT_BIN:-"$QT_DIR/$QT_VERSION/bin"}
QT_CLANG64_BIN=${QT_CLANG64_BIN:-"$QT_DIR/$QT_VERSION/clang_64/bin"}
QTFI_BIN=${QTFI_BIN:-"$QT_DIR/Tools/QtInstallerFramework/4.0/bin"}

if [ -f "$QT_BIN/qmake" ] ; then
  QMAKE=$QT_BIN/qmake
elif [ -f "$QT_CLANG64_BIN/qmake" ] ; then
  QMAKE=$QT_CLANG64_BIN/qmake
else
  die "Unable to locate qmake. Please ensure the QT_BIN and/or QT_CLANG64_BIN environment variables are set correctly for your system."
fi

if [ -f "$QT_BIN/macdeployqt" ] ; then
  MACDEPLOYQT=$QT_BIN/macdeployqt
elif [ -f "$QT_CLANG64_BIN/macdeployqt" ] ; then
  MACDEPLOYQT=$QT_CLANG64_BIN/macdeployqt
else
  die "Unable to locate macdeployqt. Please ensure the QT_BIN and/or QT_CLANG64_BIN environment variables are set correctly for your system."
fi

if [ -f "$QTFI_BIN/binarycreator" ] ; then
  BINARYCREATOR="$QTFI_BIN/binarycreator"
else
  die "Unable to locate binarycreator. Please ensure the QTFI_BIN environemnt variable is set correctly for your system."
fi

echo -e "Using binaries:\n\tqmake: $QMAKE\n\tmacdeployqt: $MACDEPLOYQT\n\tbinarycreator: $BINARYCREATOR\n"


# TODO: make it safe to run this script from any directory
cd ..
clean_old_artifacts
setup_build

# execute in a sub-shell to avoid winding up in a weird directory if the build fails.
cd build-clang_64bit-Release
$QMAKE ../src/DMHelper.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler
make -j4

cp -R DMHelper.app/ ../bin64/packages/com.dmhelper.app/data/DMHelper.app/
cp ../src/bin-macos/Info.plist ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/
mkdir ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/pkgconfig
cp ../src/bin-macos/pkgconfig/* ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/pkgconfig
mkdir ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/plugins
cp -R ../src/bin-macos/vlc/plugins/* ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/Frameworks/plugins

cd ../bin64/packages/com.dmhelper.app/data
#mkdir DMHelper.app/Contents/Frameworks
#cp -R ../../../../src/vlc64/VLCKit.framework DMHelper.app/Contents/Frameworks/VLCKit.framework
#otool -L DMHelper.app/Contents/MacOS/DMHelper
#install_name_tool -id @executable_path/../Frameworks/VLCKit.framework/Versions/A/VLCKit DMHelper.app/Contents/Frameworks/VLCKit.framework/Versions/A/VLCKit
#install_name_tool -change @loader_path/../Frameworks/VLCKit.framework/Versions/A/VLCKit @executable_path/../Frameworks/VLCKit.framework/Versions/A/VLCKit DMHelper.app/Contents/MacOS/DMHelper
otool -L DMHelper.app/Contents/MacOS/DMHelper

cd ../../../../build-clang_64bit-Release

$MACDEPLOYQT ../bin64/packages/com.dmhelper.app/data/DMHelper.app/

cp -R ../src/bestiary/ ../src/resources/ ../src/doc/ ../src/bin-macos/DMHelper.icns ../bin64/packages/com.dmhelper.app/data/DMHelper.app/Contents/Resources

cd ../bin64

message "Create the installer"

cd ../bin64
$BINARYCREATOR -c ./config/config_mac.xml -p ./packages "DM-Helper-macOS-release-Installer"
mv "./DM-Helper-macOS-release-Installer.app" "../DMHelper macOS release Installer.app"


message "Create the zip-file distribution"

cd packages/com.dmhelper.app/data
zip -r "DMHelper macOS release.zip" *
mv "DMHelper macOS release.zip" ../../../../
cd ../../../..
message "Build complete"
