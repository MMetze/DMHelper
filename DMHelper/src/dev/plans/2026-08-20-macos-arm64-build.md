# macOS ARM64 (Apple Silicon) Native Build Plan

**Goal:** Re-enable the Apple Silicon CI build by adding a native arm64 macOS build alongside the existing x86_64 build, mirroring the Windows x64/ARM64 dual-architecture pattern.

**Background:** All currently bundled macOS VLC binaries (`src/vlcMac`, `src/bin-macos`) are thin x86_64-only Mach-O (VLC 4 nightly, `x86_64-apple-darwin19` contribs). The `apple-silicon` CI leg was disabled on 2026-08-20 because a native arm64 link against x86_64 `libvlc.dylib` fails. `CMAKE_OSX_ARCHITECTURES` is currently forced to `x86_64` for all Apple hosts in `src/CMakeLists.txt`.

**Pattern to mirror (Windows):** per-arch build scripts (`buildanddeploy_msvc64_cmake.ps1` / `buildanddeploy_msvcarm64_cmake.ps1`), per-arch build dirs (`build-64_bit-release` / `build-arm64-release`), per-arch output dirs (`bin64` / `binarm64`), per-arch src payload dirs (`src/bin-win64` / `src/bin-winarm64`), per-arch VLC import dirs (`vlc64` / `vlcArm64`), arch selected via `-DDMH_WINDOWS_ARCH=...` cache variable.

**Decisions taken (2026-08-20):**
- Output/deploy dirs follow the Windows pattern: per-arch, at `DMHelper/` level.
- arm64 VLC 4 binaries are already available locally (human supplies them; agents never touch pre-built binary dirs).
- When `DMH_MACOS_ARCH` is not passed, default to **host-native** architecture.
- Legacy `DMHelper.pro` (qmake) is left as-is; CMake is the only supported build.
- Universal (fat) binaries are out of scope; two separate app bundles are distributed.

---

## Directory layout (target state)

| Purpose | x86_64 | arm64 |
|---|---|---|
| VLC headers + link dylibs | `src/vlcMac64` (renamed from `src/vlcMac`) | `src/vlcMacArm` (new) |
| Runtime payload (dylibs, plugins, pkgconfig, Info.plist, icns) | `src/bin-macos64` (renamed from `src/bin-macos`) | `src/bin-macosarm` (new) |
| Build script | `buildanddeploy_macos64_cmake.sh` (renamed) | `buildanddeploy_macosarm_cmake.sh` (new) |
| CMake build dir | `build-macos64-release` | `build-macosarm-release` |
| Deploy output dir | `DMHelper/bin-macos64` | `DMHelper/bin-macosarm` |
| Release zip | `DMHelper-macOS-x64-release.zip` | `DMHelper-macOS-arm64-release.zip` |

---

## Tasks

### Task 1: Rename x86_64 directories (human or plain `git mv`, no content changes)

- `git mv DMHelper/src/vlcMac DMHelper/src/vlcMac64`
- `git mv DMHelper/src/bin-macos DMHelper/src/bin-macos64`

No file contents change. Everything referencing the old names is updated in
Tasks 3–6 in the same change set so the build never breaks mid-sequence.

### Task 2: Populate arm64 directories (human task — pre-built binaries)

Create and fill from the locally available arm64 VLC 4 build:

- `src/vlcMacArm/` — VLC public headers (`vlc/*.h`) + `libvlc.dylib`,
  `libvlccore.dylib`
- `src/bin-macosarm/` — `libvlc.dylib`, `libvlccore.dylib`,
  `vlc/plugins/**`, `pkgconfig/`, `Info.plist`, `DMHelper.icns`

Only the **unversioned** names are required: the embedded install names are
`@rpath/libvlc.dylib` / `@rpath/libvlccore.dylib`, `-lvlc` links against
`libvlc.dylib`, and nothing references `libvlc.12.dylib` /
`libvlccore.9.dylib` by name. Rename the versioned real binaries to the
unversioned names (~200 KB / ~2 MB files — not the tiny symlink stubs the
extracted package may also contain; discard those). Versioned duplicates are
optional and add nothing.

Acceptance checks (run on a Mac or via Mach-O header inspection):
- `lipo -archs libvlc.dylib` reports `arm64` (or fat containing arm64)
- Library ABI matches the x86_64 set (sourced from `libvlc.12` /
  `libvlccore.9`, same VLC 4 API — `libvlc_media_player_stop_async`,
  single-arg `libvlc_media_new_path`)
- Plugins are arm64 and reference `@rpath/libvlccore.dylib`
- `Info.plist` for arm64 sets `LSMinimumSystemVersion` to `11.0`
  (arm64 macOS floor; the copied 10.12 value is wrong for both arches —
  x86_64 contribs are darwin19 = 10.15, fix opportunistically)

**Windows-machine handling (files are copied/committed from Windows):**

- The existing repo stores the unversioned `libvlc.dylib` / `libvlccore.dylib`
  as git *symlinks* (15/18-byte stubs on Windows checkout). Git on Windows
  (default `core.symlinks=false`) cannot create new symlink entries — do
  **not** try to replicate them. Ship the real binaries directly under the
  unversioned names as described above.
- Prefer the `.tar.xz` VLC package over the `.dmg` — Windows extraction
  tools handle it faithfully; `.dmg` extraction can silently drop or stub
  symlinked entries.
- Verify before pushing (from the new payload dirs, PowerShell):
  - No symlink stubs snuck in:
    `Get-ChildItem -Recurse -Filter *.dylib | Where-Object Length -lt 4096`
    must return nothing.
  - Spot-check Mach-O headers: first bytes `CF FA ED FE` + `0C 00 00 01`
    at offset 4 = thin arm64; `CA FE BA BE` = fat. (The x86_64 set reads
    `07 00 00 01`.)
- Binary content itself is safe through git from Windows: Mach-O files are
  NUL-heavy so git treats them as binary (no CRLF mangling); embedded code
  signatures survive byte-copies; lost execute bits and xattrs are harmless
  (dyld needs neither, CI re-signs ad-hoc).

### Task 2b: Add `.gitattributes` for binary payloads

The repo has no `.gitattributes`. Add one at the repo root so no
contributor's `core.autocrlf` setting can ever corrupt payload files:

```gitattributes
*.dylib -text
*.icns  -text
*.dll   -text
*.lib   -text
*.a     -text
*.dat   -text
```

(Defense-in-depth: git's NUL-byte auto-detection already protects these
today; this makes it explicit and covers any future edge case.)

### Task 3: CMake architecture selection (`src/CMakeLists.txt`)

Replace the current hard-coded x86_64 guard (lines 3–9) with a
`DMH_MACOS_ARCH` cache variable mirroring `DMH_WINDOWS_ARCH`:

- **Before `project()`** (guarded by `CMAKE_HOST_APPLE`):
  - `DMH_MACOS_ARCH=x64` → `set(CMAKE_OSX_ARCHITECTURES "x86_64")`
  - `DMH_MACOS_ARCH=arm64` → `set(CMAKE_OSX_ARCHITECTURES "arm64")`
  - unset → leave `CMAKE_OSX_ARCHITECTURES` alone (host-native default)
- **In the `elseif (APPLE)` block**: select VLC dirs from the *effective*
  architecture — use `DMH_MACOS_ARCH` if defined, otherwise
  `CMAKE_SYSTEM_PROCESSOR` (`arm64` → `vlcMacArm`, else `vlcMac64`) —
  replacing the hard-coded `vlcMac` include/link dirs. Emit
  `message(STATUS "Using <dir> VLC")` like the Windows branch does.

### Task 4: Build scripts

- Rename `buildanddeploy_macos_cmake.sh` → `buildanddeploy_macos64_cmake.sh`:
  - `BUILD_DIR` → `build-macos64-release`, `BIN_DIR` → `$SCRIPT_ROOT/bin-macos64`
  - Payload copies read from `$SRC_DIR/bin-macos64/...`
  - Configure passes `-DDMH_MACOS_ARCH=x64`
  - Zip name → `DMHelper-macOS-x64-release.zip`
- Create `buildanddeploy_macosarm_cmake.sh` (copy of the above with):
  - `build-macosarm-release`, `bin-macosarm`, `$SRC_DIR/bin-macosarm/...`
  - `-DDMH_MACOS_ARCH=arm64`
  - Zip name → `DMHelper-macOS-arm64-release.zip`
- Both keep the `--skip-build` flag and the `echo "y" |` confirmation flow
  used by CI.

### Task 5: CI workflow (`.github/workflows/dmh-build-mac.yml`)

Re-enable the `apple-silicon` matrix leg and extend the matrix:

```yaml
include:
  - runner: macos-15-intel
    arch-label: intel
    build-script: buildanddeploy_macos64_cmake.sh
    bin-dir: bin-macos64
    zip-name: DMHelper-macOS-x64-release.zip
  - runner: macos-15
    arch-label: apple-silicon
    build-script: buildanddeploy_macosarm_cmake.sh
    bin-dir: bin-macosarm
    zip-name: DMHelper-macOS-arm64-release.zip
```

Replace every hard-coded `bin-macos/DMHelper.app`, script name, and zip/dmg
filename with the matrix variables (four `APP=` sites: codesign, Gatekeeper,
repackage, smoke test; plus chmod and both upload steps). The smoke test runs
natively on each runner — no Rosetta involvement.

Note: an arm64 bundle **must** carry a valid signature to launch; the existing
ad-hoc `codesign --force --deep --sign -` step covers this, and the plugins
under `Contents/Frameworks/plugins` are included via `--deep`. Keep the
codesign step ordered after the last file is copied into the bundle.

### Task 6: Documentation / agent-instruction updates

Update forbidden-folder and pre-built-binary lists to the new names
(`vlcMac64/`, `vlcMacArm/`, `bin-macos64/`, `bin-macosarm/`):

- `.github/copilot-instructions.md`
- `CLAUDE.md`
- `.github/agents/execution.agent.md`
- `.github/agents/review.agent.md`
- `doc/DM Helper Ruleset and Templates.md` (+ regenerate/patch the `.html`)
- The explanatory comment at the top of `src/CMakeLists.txt` (removed by Task 3)

`DMHelper.pro` is intentionally **not** updated (legacy qmake, unsupported).

### Task 7: Verification

- CI: both matrix legs green; both artifacts uploaded (zip + dmg per arch).
- Confirm binary arch inside each artifact
  (`lipo -archs Contents/MacOS/DMHelper`).
- Smoke test passes on both runners (already enforced by workflow).
- Manual on Apple Silicon hardware when available: launch the arm64 bundle,
  load a campaign with a `LayerVideo`, verify DM-side screenshot
  (`VideoPlayerScreenshot`) and player-window publish (`VideoPlayer`
  non-GL path) play without crashing. This also gives the first data point
  for the separate open issue: the x86_64-under-Rosetta crash at video load
  (still unexplained; crash report wanted — likely VideoToolbox-under-Rosetta
  or AVX in nightly contribs, and would not affect the native arm64 build).

---

## Risks / notes

- **Version skew between the two VLC payloads.** If the arm64 nightly is a
  different VLC revision than the bundled x86_64 one, plugin/core ABI could
  differ per arch. Same-revision downloads strongly preferred (Task 2 check).
- **`libvlc_new(0, nullptr)` plugin discovery** relies on dladdr →
  `Contents/Frameworks/plugins`; the deploy layout is identical for both
  arches, so no code change in `dmh_vlc.cpp` is expected. The plugin-cache
  logic remains `#ifndef Q_OS_MAC` — unchanged.
- **No C++ source changes are anticipated.** `videoplayer.cpp` /
  `layervideo.cpp` are arch-agnostic; the only platform-specific path
  (backslash rewrite) is Windows-only.
- **Stale CMake caches**: renaming dirs invalidates `build-macos-release`;
  the per-arch build dirs avoid cross-arch cache pollution by construction.
- **Symlink hazard when adding files from Windows**: a symlink stub committed
  as a regular file checks out on macOS as a text file named `.dylib`; dyld
  then fails to resolve `@rpath/libvlc.dylib` and the app won't launch.
  Task 2's stub check catches this before push.
- Renames of pre-built binary dirs are performed by the human (or plain
  `git mv`) — agents must not modify the contents of those directories.
