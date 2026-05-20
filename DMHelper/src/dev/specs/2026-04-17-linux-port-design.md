# DMHelper Linux Port — Design Spec

## Goal

Port DMHelper to Linux with full feature parity to the Windows version. Distribute as a self-contained AppImage that bundles Qt and VLC libraries so users need no external dependencies.

## Current State

- Qt 6 / C++17 / CMake — ~95% of code is platform-independent Qt
- Platform-specific code limited to VLC integration, file paths, and minor OS conditionals
- GitHub Actions Linux workflow exists but needs rework for cmake
- Windows and macOS are fully supported with bundled VLC
- Build system is CMakeLists.txt (primary), .pro file exists but is not the focus

## Section 1: Platform-Specific Code Fixes

### Qt 6.4 Compatibility (Ubuntu ships Qt 6.4, not 6.6)

- `QMediaPlayer::isPlaying()` was added in Qt 6.5. Replace with
  `playbackState() == QMediaPlayer::PlayingState` in `soundboardframe.cpp`,
  `combatantwidgetcharacter.cpp`, `combatantwidgetmonster.cpp`.
- Qt 6.4's MOC requires full type definitions for types used in signal
  parameters. Forward declarations are insufficient — the MOC-generated code
  references the type's size and structure. Affected headers (~15 files) must
  `#include` the full header instead of forward-declaring. This only applies
  to types that appear in `signals:` blocks.

### VLC 3.0 Compatibility (Ubuntu ships VLC 3.x, not 4.x)

- `libvlc_media_player_stop_async()` is VLC 4.0 API. Replace with
  `libvlc_media_player_stop()` (synchronous, VLC 3.0) in `videoplayer.cpp`,
  `videoplayerglplayer.cpp`, `videoplayerglscreenshot.cpp`,
  `audiotrackyoutube.cpp`.
- VLC 4.0 changed the GL rendering callback signatures. Guard the new API
  behind `#if LIBVLC_VERSION_INT >= LIBVLC_VERSION(4,0,0,0)` in
  `videoplayerglplayer.cpp`, `videoplayerglscreenshot.cpp`,
  `videoplayerglvideo.cpp/.h`.
- Existing `#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)` VLC API
  conditionals already fall through to the old API via `#else`, which is
  correct for Linux + VLC 3.x. No changes needed there.

### File Paths

- `QUrl("path/to/file.mp3")` doesn't resolve correctly on Linux for local
  files. Must use `QUrl::fromLocalFile()` in `audiotrackfile.cpp`.
- Linux data directory: `~/.local/share/DMHelper/` (via `QStandardPaths`)
- Linux config directory: `~/.config/DMHelper/` (via `QStandardPaths`)

### Windows Headers

- `BaseTsd.h` and `ssize_t` in `dmh_vlc.h` — already behind `#ifdef Q_OS_WIN`.
  No change needed.

### Application Icon

- Existing `data/dmhelper.png` is used for Linux desktop integration.

## Section 2: Build System Changes (CMakeLists.txt)

All changes are additive — no modifications to existing Windows or Mac blocks.

### Linux Platform Block

Add an `elseif (UNIX AND NOT APPLE)` block to CMakeLists.txt:

- Compiler flags: `-g` for debug, `-O2` for release
- VLC: `target_include_directories` for `/usr/include` and `/usr/include/vlc`,
  `target_link_libraries` with `vlc`
- RPATH: `INSTALL_RPATH "$ORIGIN/lib"` for AppImage self-containment

### Local Build Script

`build-linux.sh` installs apt dependencies, runs cmake, and deploys
resources/bestiary/doc alongside the binary.

## Section 3: GitHub Actions CI Pipeline

### Build Environment

- Ubuntu 22.04
- Qt 6.6.0 via `jurplel/install-qt-action@v3`
- System packages: `libvlc-dev`, `libvlccore-dev`, `vlc-plugin-base`,
  `libgl1-mesa-dev`, X11/XCB libs, FUSE (for AppImage)

### Build Steps

1. `cmake -S DMHelper/src -B build -DCMAKE_BUILD_TYPE=Release`
2. `cmake --build build --config Release -j$(nproc)`

### AppImage Packaging

- Use `linuxdeploy` with the Qt plugin to bundle Qt libraries/plugins
- Copy VLC libraries and plugins from system
- Custom `AppRun.sh` sets `VLC_PLUGIN_PATH` before launching binary
- Produce a single `.AppImage` artifact

### Triggers

- Push to `master` or `feature/linux-port`
- Pull requests to `master`
- Manual dispatch

## Section 4: AppImage Structure & Desktop Integration

### AppImage Contents

```
DMHelper.AppImage
├── AppRun (custom — sets VLC_PLUGIN_PATH)
├── DMHelper.desktop
├── DMHelper.png
├── usr/
│   ├── bin/
│   │   ├── DMHelper
│   │   ├── resources/
│   │   ├── bestiary/
│   │   └── doc/
│   ├── lib/
│   │   ├── libQt6*.so
│   │   ├── libvlc.so*
│   │   ├── libvlccore.so*
│   │   └── vlc/plugins/
│   └── plugins/ (Qt plugins)
```

### Desktop Entry File

```ini
[Desktop Entry]
Type=Application
Name=DMHelper
GenericName=D&D Campaign Manager
Comment=Dungeons & Dragons campaign management tool
Exec=DMHelper
Icon=DMHelper
Categories=Game;RolePlaying;
Terminal=false
StartupNotify=true
```

### Runtime Configuration

- `VLC_PLUGIN_PATH` set in AppRun to point to bundled VLC plugins
- `LD_LIBRARY_PATH` extended to include bundled libraries
- User data: `~/.local/share/DMHelper/`
- Config: `~/.config/DMHelper/`

## Out of Scope (this PR)

- YouTube audio via yt-dlp (will be a separate PR)
- Wayland-native support (X11/XWayland is sufficient)
- Linux-specific features beyond parity
- Flatpak or `.deb` packaging (AppImage only)
- AI-generated or externally-sourced artwork/assets
