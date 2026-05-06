# DMHelper Project Guidelines

## Architecture
Desktop VTT for DMs. C++17/Qt 6. Dual-window: DM view + OpenGL player screen.
All active code is in `DMHelper/src/`. Folders `DMHelper-Backend/`, `DMHelperClient/`,
`DMHelperShared/`, and `DMHelperTest/` are archived — never touch them.

## Build
Always wrap cmake in a `vcvarsall.bat` call — plain PowerShell does not have MSVC paths.
```powershell
cmd /c "call ""C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"" x64 > nul 2>&1 && cd /d c:\Users\turne\Documents\GitHub\DMHelper && C:\Qt\Tools\CMake_64\bin\cmake.exe --build DMHelper/out/build/windows-debug 2>&1"
```
`cmake` is at `C:\Qt\Tools\CMake_64\bin` — not on the system PATH.

IntelliSense errors in `.cpp` files are unreliable false positives — verify correctness via the actual cmake build.

## Source List
Sources are listed **explicitly** in `CMakeLists.txt` — no globbing. Every new `.cpp`/`.h` pair must be added to the source list in the same change. Never create a `.cpp` without a corresponding `CMakeLists.txt` update.

## Files Never to Modify
- `*.ui` — Qt Designer only; never hand-edit the XML
- `*.qrc` — manual edits only; never restructure paths
- `vlc32/`, `vlc64/`, `vlcMac/`, `bin-win*/`, `bin-macos/` — pre-built binaries

## Signals
- `dirty()` = unsaved data changed. `changed()` = visual-only redraw.
- Never emit `dirty()` in constructors or `inputXML()`.

## Serialisation
Override `createOutputXML()` + `internalOutputXML()`. Always call the base class. Use `postProcessXML()` for cross-references — never `inputXML()`.

## Disabled Feature Flags (do not enable without explicit discussion)
- `INCLUDE_NETWORK_SUPPORT` — network stack incomplete
- `LAYERVIDEO_USE_OPENGL` — GPU video path not production-ready

## UI Creation
Dialogs and frames derive from `.ui` files designed in Qt Designer. Do not override `.ui` properties (margins, spacing, stylesheets, size policies) from code. If a UI property needs changing, describe what to adjust in Qt Designer and let the user make the change. Programmatic widget creation is only acceptable for runtime-data-driven widgets (e.g. populating a conditions grid from the active ruleset); even then the shell layout must come from the `.ui` file.

## Agent Workflow
- Branch: `agent/work` — never commit to `main`
- Commit after each logical unit: `agent: <what changed>`
- After non-trivial changes, run the build command above to verify
