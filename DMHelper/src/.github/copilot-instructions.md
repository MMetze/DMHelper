# DMHelper Project Guidelines

## Architecture
Desktop VTT for DMs. C++17/Qt 6. Dual-window: DM view + OpenGL player screen.
All active code is in `DMHelper/src/`. Folders `DMHelper-Backend/`, `DMHelperClient/`,
`DMHelperShared/`, and `DMHelperTest/` are archived — never touch them.

## Build
Use the CMake Tools extension directly for configure/build/test tasks.
- Build with `Build_CMakeTools`
- Configure with the same CMake preset flow if the build tree is missing
- Run tests with `RunCtest_CMakeTools`

Do not use manual `cmake` terminal commands for routine builds in this workspace.

IntelliSense errors in `.cpp` files are unreliable false positives — verify correctness via the actual cmake build.

## Source List
Sources are listed **explicitly** in `CMakeLists.txt` — no globbing. Every new `.cpp`/`.h` pair must be added to the source list in the same change. Never create a `.cpp` without a corresponding `CMakeLists.txt` update.

## Files Never to Modify
- `*.ui` — Qt Designer only; never hand-edit the XML
- `*.qrc` — manual edits only; never restructure paths
- `vlc32/`, `vlc64/`, `vlcMac64/`, `vlcMacArm/`, `bin-win*/`, `bin-macos64/`, `bin-macosarm/` — pre-built binaries

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
