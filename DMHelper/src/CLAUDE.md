# CLAUDE.md — DMHelper

## What this is
Desktop VTT for DMs. C++17/Qt 6. Dual-window: DM view + OpenGL player screen.
Win64 + macOS. All active code is in `DMHelper/src/`. Everything in
`DMHelper-Backend/`, `DMHelperClient/`, `DMHelperShared/`, `DMHelperTest/`
is archived — do not touch.

## Build

Default: use the `windows-debug` preset through the CMake Tools extension for day-to-day development.

Use the CMake Tools extension directly for configure/build/test tasks.
- Build with `Build_CMakeTools`
- Configure with the same preset flow if the build tree is missing
- Run tests with `RunCtest_CMakeTools`

Do not use manual `cmake` terminal commands for routine builds in this workspace.

**IntelliSense errors are unreliable.** The IDE's clangd/IntelliSense uses a
different Qt configuration than the actual build. Errors like "no type named
'in_place_t'", "Cannot initialize a parameter of type 'QDialog *'", and
"'QFlagsStorageHelper' is a protected member" are false positives. Do not
attempt to fix them — verify correctness by running the actual cmake build
instead. The `get_errors` tool output should be ignored for `.cpp` files in
this project.

Sources are listed **explicitly** in `CMakeLists.txt` — no globbing. If you
create a `.cpp`/`.h` pair, you must add both to the source list manually.

## Conventions — follow existing code, but note these traps

**No magic numbers.** Use named constants (`static constexpr` for scalars,
`static const` for QColor etc.) at the top of the `.cpp` file for any
non-trivial literal value. Default property values, timer intervals, buffer
sizes, vertex layout parameters, preview sizes, gradient stops — all must be
named. The only exceptions are:
- Structural zeros/ones with obvious meaning (e.g. `0.0f` for a cleared
  position, `1` for a boolean attribute default)
- Values that are part of a well-known algorithm embedded in a string
  literal (e.g. simplex noise constants inside GLSL shader strings)

**Enums** use `TypeName_ValueName` (e.g. `LayerType_Fow`, `CampaignType_Battle`).
This is non-obvious — don't drift to `LayerType::Fow` or plain `FOW`.

**Dirty vs changed signals:** `dirty()` = unsaved data changed.
`changed()` = visual-only. Never emit `dirty()` in constructors or `inputXML()`.

**Serialisation:** Override `createOutputXML()` + `internalOutputXML()`.
Always call the base class implementation. Use `postProcessXML()` for
cross-references — not `inputXML()`.

**Layer subclasses** must implement both interfaces independently:
- DM path: `dmInitialize / dmUninitialize / dmUpdate`
- Player path: `playerGLInitialize / playerGLUninitialize / playerGLPaint`
Neither path may assume the other is active.

## Threading — this is where crashes happen

libvlc decode callbacks (`lockCallback`, `unlockCallback`, `displayCallback`)
run on VLC's internal thread — **never touch Qt GUI objects from them**.
Marshal everything back via:
```cpp
QMetaObject::invokeMethod(obj, ..., Qt::QueuedConnection);
```
`Qt::AA_DontCheckOpenGLContextThreadAffinity` in `main.cpp` is intentional —
`VideoPlayerGLVideo` calls `makeCurrent()` from the VLC thread. Do not remove it.

## New files
If a task requires creating new .cpp/.h files, Claude must:
1. State what files it intends to create and why, before creating them
2. Add them to the explicit source lists in CMakeLists.txt in the same commit
3. Never create a .cpp file without a corresponding CMakeLists.txt update

## GL context rules — critical
OpenGL calls require an active GL context. This is ONLY guaranteed in 
functions with "GL" in their name (playerGLInitialize, playerGLPaint, etc.).

Never make GL calls from:
- Constructors or destructors
- inputXML() or outputXML()  
- Qt signal handlers
- activateObject() / deactivateObject()
- Any function without "GL" in its name

Shader initialisation uses lazy-loading: shaders are created in 
playerGLInitialize() where possible, with a fallback guard at the top 
of playerGLPaint():
  if(_shaderProgramRGBA == 0) createShaders();
This guard is intentional — do not remove it.

## Legacy classes — always use v2

| Wrong | Right |
|---|---|
| `MonsterClass` | `MonsterClassv2` |
| `Character` | `Characterv2` |

Legacy classes remain only for XML file compatibility. New code uses v2 exclusively.

## Disabled feature flags — do not enable without discussion
- `INCLUDE_NETWORK_SUPPORT` — network stack incomplete, gated in `dmconstants.h`
- `LAYERVIDEO_USE_OPENGL` — GPU video path exists but not production-ready

## UI creation — prefer .ui files

Minimise programmatic UI creation. Dialogs and frames should always derive
from a `.ui` file designed by the user in Qt Designer. The only exception
is truly dynamic content whose structure depends on runtime data (e.g.
populating a grid of condition buttons from the active ruleset). Even then,
the **shell** (scroll areas, button boxes, static labels, layout structure)
must come from the `.ui` file — only the data-driven widgets are added in code.

**Never override .ui properties from code.** Layout margins, spacing, size
policies, stylesheets, and other visual properties that can be set in
Qt Designer must be set there — not overridden programmatically in the
constructor or elsewhere. If a .ui property needs changing, tell the user
which widget and property to adjust in Qt Designer and let them make the
change. The only acceptable programmatic UI modifications are those driven
by runtime state (e.g. showing/hiding widgets, updating text, populating
dynamic lists).

## Files never to modify
- `*.ui` — edit in Qt Designer only, never hand-edit XML
- `*.qrc` — edit manually only with care, never restructure paths
- `vlc32/`, `vlc64/`, `vlcMac64/`, `vlcMacArm/`, `bin-win*/`, `bin-macos64/`, `bin-macosarm/` — pre-built binaries

## Agent workflow
- Branch: `agent/work` — never commit directly to `main`
- Commit after each logical file/unit: `agent: <what changed>`
- After non-trivial changes, run the build command above to verify
- Keep tasks to one subsystem at a time
- Before asking permission to run a script or external command, summarize what the command does and why it is needed