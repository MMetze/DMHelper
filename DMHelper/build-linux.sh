#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")" && git rev-parse --show-toplevel)"
VLC_SNAP_URL="https://artifacts.videolan.org/vlc/nightly-snap/20260420-0440/vlc_4.0.0-dev-36688-g3462acc0a9_amd64.snap"

echo "=== Installing dependencies ==="
sudo apt-get update
sudo apt-get install -y \
  libgl1-mesa-dev \
  libxkbcommon-dev \
  build-essential \
  cmake \
  squashfs-tools \
  yt-dlp

echo "=== Fixing VLC symlinks (git stores them as text files on Windows) ==="
cd "$REPO_ROOT/DMHelper/src/vlclinux"
rm -f libvlc.so libvlc.so.12 libvlccore.so libvlccore.so.9
ln -s libvlc.so.12.0.0 libvlc.so
ln -s libvlc.so.12.0.0 libvlc.so.12
ln -s libvlccore.so.9.0.0 libvlccore.so
ln -s libvlccore.so.9.0.0 libvlccore.so.9

echo "=== Building DMHelper ==="
cd "$REPO_ROOT"
rm -rf build-release
cmake -S DMHelper/src -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release -j$(nproc)

echo "=== Deploying resources ==="
cd build-release
cp -r "$REPO_ROOT/DMHelper/src/resources" .
cp -r "$REPO_ROOT/DMHelper/src/doc" .
mkdir -p resources
cp "$REPO_ROOT/DMHelper/src/bestiary/"*.xml resources/

echo "=== Deploying VLC 4 libraries ==="
mkdir -p lib
cp "$REPO_ROOT/DMHelper/src/vlclinux/libvlc.so.12.0.0" lib/
cp "$REPO_ROOT/DMHelper/src/vlclinux/libvlccore.so.9.0.0" lib/
cd lib
ln -sf libvlc.so.12.0.0 libvlc.so.12
ln -sf libvlc.so.12.0.0 libvlc.so
ln -sf libvlccore.so.9.0.0 libvlccore.so.9
ln -sf libvlccore.so.9.0.0 libvlccore.so
cd ..

echo "=== Deploying VLC 4 plugins ==="
if [ ! -d "$HOME/squashfs-root/usr/lib/vlc/plugins" ]; then
    echo "Downloading VLC 4 snap (one-time download)..."
    cd "$HOME"
    wget -q "$VLC_SNAP_URL"
    unsquashfs -q *.snap
    rm -f *.snap
    cd "$REPO_ROOT/build-release"
fi
mkdir -p lib/vlc/plugins
cp -r "$HOME/squashfs-root/usr/lib/vlc/plugins/"* lib/vlc/plugins/

echo "=== Creating launcher script ==="
cat > DMHelper.sh << 'LAUNCHER'
#!/bin/bash
HERE="$(cd "$(dirname "$0")" && pwd)"
export VLC_PLUGIN_PATH="${HERE}/lib/vlc/plugins"
export LD_LIBRARY_PATH="${HOME}/Qt/6.6.0/gcc_64/lib:${HERE}/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/DMHelper" "$@"
LAUNCHER
chmod +x DMHelper.sh

echo ""
echo "=== Build complete! ==="
echo ""
echo "Run with:"
echo "  cd $REPO_ROOT/build-release && ./DMHelper.sh"
