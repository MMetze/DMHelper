---
description: "Use when writing, editing, or reviewing C++ or Qt code in DMHelper. Covers naming conventions, enum style, magic-number policy, OpenGL context rules, threading constraints, layer interfaces, and legacy class preferences."
applyTo: ["**/*.cpp", "**/*.h"]
---

# C++ / Qt Coding Standards — DMHelper

## Naming

- **Enums** use `TypeName_ValueName` (e.g. `LayerType_Fow`, `CampaignType_Battle`).
  Never use `TypeName::ValueName` or plain `UPPER_SNAKE`.
- **Legacy classes**: always use the v2 counterpart:
  | Wrong | Right |
  |---|---|
  | `MonsterClass` | `MonsterClassv2` |
  | `Character` | `Characterv2` |

## Magic Numbers
Use named constants at the top of the `.cpp` file for every non-trivial literal:
- `static constexpr` for scalars (intervals, sizes, counts, ratios)
- `static const` for `QColor`, `QSize`, `QPointF`, etc.

Exceptions:
- Structural zeros/ones with obvious meaning (`0.0f` cleared position, `1` boolean default)
- Constants embedded inside GLSL shader string literals (e.g. simplex noise coefficients)

## Signals
- `dirty()` — unsaved data changed. `changed()` — visual-only redraw.
- **Never emit `dirty()`** in constructors or `inputXML()`.

## Serialisation
- Override `createOutputXML()` + `internalOutputXML()`. **Always call the base class.**
- Use `postProcessXML()` for cross-references — never `inputXML()`.

## OpenGL Context Rules
OpenGL calls require an active context. Context is **only** guaranteed in functions with `GL` in their name (`playerGLInitialize`, `playerGLPaint`, etc.).

**Never make GL calls from:**
- Constructors or destructors
- `inputXML()` or `outputXML()`
- Qt signal handlers
- `activateObject()` / `deactivateObject()`
- Any function without `GL` in its name

Shader initialisation uses lazy-loading: shaders are created in `playerGLInitialize()` and guarded at the top of `playerGLPaint()`:
```cpp
if (_shaderProgramRGBA == 0) createShaders();
```
Do not remove this guard.

## Threading — VLC Callbacks
`lockCallback`, `unlockCallback`, and `displayCallback` run on VLC's internal thread.
**Never touch Qt GUI objects from them.** Marshal back via:
```cpp
QMetaObject::invokeMethod(obj, ..., Qt::QueuedConnection);
```
`Qt::AA_DontCheckOpenGLContextThreadAffinity` in `main.cpp` is intentional — do not remove it.

## Layer Subclasses
Must implement both paths independently:
- **DM path**: `dmInitialize / dmUninitialize / dmUpdate`
- **Player path**: `playerGLInitialize / playerGLUninitialize / playerGLPaint`

Neither path may assume the other is active.

## New Files
When creating a new `.cpp`/`.h` pair:
1. State the files and rationale before creating them.
2. Add both to the explicit source list in `CMakeLists.txt` in the same change.
