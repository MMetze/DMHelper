# DMHelper Linux Port Implementation Plan

**Goal:** Port DMHelper to Linux with full feature parity and distribute as a self-contained AppImage bundling Qt and VLC.

**Architecture:** The codebase is Qt 6 / C++17 with ~95% platform-independent code. The port requires: (1) compatibility fixes for Qt 6.4 and VLC 3.0 (Ubuntu's versions), (2) adding a Linux platform block to CMakeLists.txt, (3) rewriting the GitHub Actions Linux workflow for cmake, and (4) creating AppImage packaging with bundled VLC.

**Tech Stack:** Qt 6, C++17, CMake, libvlc, OpenGL, GitHub Actions, linuxdeploy (AppImage tooling)

**Spec:** `dev/specs/2026-04-17-linux-port-design.md`

**Base branch:** `origin/v3.8.1`

---

## Completed Tasks

### Task 1: Add Linux OS Identification in Startup Logging

**Files:** `DMHelper/src/mainwindow.cpp`

Added `#elif defined(Q_OS_LINUX)` to the existing OS detection block so the
startup log correctly reports "Linux" instead of falling through to "Windows".

---

### Task 2: Create Desktop Entry and AppRun Script

**Files:**
- `DMHelper/src/data/linux/DMHelper.desktop` — freedesktop .desktop entry
- `DMHelper/src/data/linux/AppRun.sh` — sets `VLC_PLUGIN_PATH` and
  `LD_LIBRARY_PATH` before launching the binary inside the AppImage

---

### Task 3: Rewrite GitHub Actions Linux CI Workflow

**Files:** `.github/workflows/dmh-build-linux.yml`

The original workflow was broken (used Windows tools on a Linux runner).
Rewrote to:
- Install system deps (VLC, GL, X11/XCB, FUSE)
- Install Qt 6.6.0 via `jurplel/install-qt-action@v3`
- Build with cmake
- Package as AppImage via linuxdeploy with custom AppRun
- Upload artifact
- Trigger on push to `master`/`feature/linux-port`, PRs to `master`, and
  manual dispatch

---

### Task 4: Add Local Linux Build Script

**Files:** `build-linux.sh`

Installs apt dependencies (Qt6, VLC, cmake), builds with cmake, and deploys
resources/bestiary/doc alongside the binary.

---

### Task 5: Qt 6.4 Compatibility — QMediaPlayer::isPlaying()

**Files:** `DMHelper/src/combatantwidgetcharacter.cpp`,
`DMHelper/src/combatantwidgetmonster.cpp`

`QMediaPlayer::isPlaying()` was added in Qt 6.5. Ubuntu 22.04/24.04 ship
Qt 6.4. Replaced with `playbackState() == QMediaPlayer::PlayingState`.

---

### Task 6: VLC 3.0 Compatibility — stop_async and GL API

**Files:** `DMHelper/src/videoplayer.cpp`, `DMHelper/src/audiotrackyoutube.cpp`,
`DMHelper/src/videoplayerglplayer.cpp`, `DMHelper/src/videoplayerglscreenshot.cpp`,
`DMHelper/src/videoplayerglvideo.cpp`, `DMHelper/src/videoplayerglvideo.h`

Two changes:
1. **stop_async → stop:** `libvlc_media_player_stop_async()` is VLC 4.0 API.
   Ubuntu ships VLC 3.0 which only has synchronous `libvlc_media_player_stop()`.
   Replaced all calls. The synchronous version is also available in VLC 4.0
   so this doesn't break Windows/Mac.
2. **GL rendering API guard:** VLC 4.0 changed GL callback signatures. Added
   `#ifdef` guards so VLC 3.0 compiles without the new callbacks while VLC 4.0
   continues to use them.

---

### Task 7: Qt 6.4 MOC Compatibility — Forward Declarations → Includes

**Files:** `DMHelper/src/battleframe.h`, `DMHelper/src/map.h`,
`DMHelper/src/mapframe.h`, `DMHelper/src/mapframescene.h`,
`DMHelper/src/layerscene.h`, `DMHelper/src/layertokens.h`,
`DMHelper/src/battledialogmodel.h`, `DMHelper/src/combatantwidget.h`,
`DMHelper/src/battlecombatantframe.h`, `DMHelper/src/soundboardframe.h`,
`DMHelper/src/soundboardtrackframe.h`,
`DMHelper/src/characterimportheroforge.h`,
`DMHelper/src/characterimportheroforgedialog.h`,
`DMHelper/src/initiativelistcombatantwidget.h`,
`DMHelper/src/ribbontabworldmap.h`

**Why these are needed:** Qt's Meta-Object Compiler (MOC) generates C++ code
for all types used in `signals:` and `slots:` declarations. On Qt 6.4
(Ubuntu's version), MOC requires the full type definition — a forward
declaration is insufficient because the generated code references the type's
size and structure for signal argument marshalling. With only a forward
declaration, MOC emits code that fails to compile with errors like
"incomplete type" or "invalid use of incomplete type".

This only affects headers where a custom type appears in a signal parameter
list. Standard Qt types (QString, int, QPointF, etc.) are fine because their
headers are already transitively included.

On Qt 6.6+ (Windows/Mac), MOC is more lenient and accepts forward
declarations in these positions. The full includes are harmless on those
platforms — they just make the dependency explicit.

---

### Task 8: Linux Runtime Fix — Local Audio File Paths

**Files:** `DMHelper/src/audiotrackfile.cpp`

On Linux, `QUrl("path/to/file.mp3")` creates a relative URL that
`QMediaPlayer` can't resolve. Replaced with `QUrl::fromLocalFile()` which
produces a proper `file:///` URL. This is a no-op on Windows where both
forms work.

---

### Task 9: Deploy Resources in Build Script

**Files:** `build-linux.sh`

Copies `resources/`, `doc/`, and bestiary XML files alongside the built
binary so template/bestiary/PDF paths resolve correctly at runtime.

---

### Task 10: Add Linux Platform Block to CMakeLists.txt

**Files:** `DMHelper/src/CMakeLists.txt`, `build-linux.sh`,
`.github/workflows/dmh-build-linux.yml`

Added `elseif (UNIX AND NOT APPLE)` block with:
- Debug/release compiler flags
- VLC include/link directories
- `INSTALL_RPATH "$ORIGIN/lib"` for AppImage self-containment

Updated build-linux.sh and CI workflow to use cmake instead of qmake.

---

## Deferred to Separate PR

### YouTube Audio via yt-dlp

Replace the internal `api.dmhh.net` backend with `yt-dlp` for YouTube URL
resolution on Linux. This is a larger change that requires:
- Decoupling `AudioTrackYoutube` from the current backend
- Adding volume and playback controls for the yt-dlp path
- Error handling and timeout management
- yt-dlp as a runtime dependency

Will be submitted as a separate PR for independent review.

---

## Testing Checklist

- [X] Build completes on Linux (Ubuntu 22.04/24.04) with cmake
- [X] Application launches and displays ribbon UI
- [X] Campaign creation and loading
- [X] Battle map rendering (OpenGL)
- [X] Character/monster sheet display
- [X] VLC video playback
- [X] Local audio file playback
- [X] AppImage runs on clean system without system Qt or VLC
- [ ] Windows and Mac CI builds remain green (no regressions)
