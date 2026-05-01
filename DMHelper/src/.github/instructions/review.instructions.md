---
description: "Use when reviewing code, performing a code review, checking for regressions, assessing risk, auditing changes, or looking for bugs in DMHelper. Returns findings-first output with risk severity and no implementation side-effects."
---

# Code Review Guidelines — DMHelper

## Output Format
Always structure review output as:

1. **Risk summary** — one sentence: overall risk level (Low / Medium / High / Critical) and primary concern.
2. **Findings** — ordered highest-risk first. For each finding:
   - Severity: `Critical | High | Medium | Low | Info`
   - Location: file + line or function name
   - Issue: what is wrong
   - Why it matters: crash / data loss / silent corruption / UX regression / style
   - Suggested fix: concrete and minimal — do not implement it
3. **What looks correct** — brief acknowledgement of sound patterns.

Do **not** implement fixes unless explicitly asked. Review is read-only analysis.

## High-Risk Areas — Always Check

### Threading
- VLC callbacks (`lockCallback`, `unlockCallback`, `displayCallback`) must never touch Qt GUI objects.
  Any direct GUI access from these is `Critical`.
- All GUI updates from non-main threads must use `Qt::QueuedConnection`.

### OpenGL Context
- GL calls outside functions named with `GL` (`playerGLInitialize`, `playerGLPaint`, etc.) are `Critical`.
- Missing lazy-load guard `if (_shaderProgramRGBA == 0) createShaders();` at top of paint functions is `High`.

### Signals
- `dirty()` emitted from a constructor or `inputXML()` is `High` (causes spurious save prompts / feedback loops).
- Ensure `dirty()` vs `changed()` distinction is respected.

### Serialisation
- `internalOutputXML()` not calling the base class is `High` (data loss on save).
- Cross-reference resolution in `inputXML()` instead of `postProcessXML()` is `Medium`.

### Archived Folders
- Any change touching `DMHelper-Backend/`, `DMHelperClient/`, `DMHelperShared/`, or `DMHelperTest/` is `Critical`.

### Legacy Classes
- New code using `MonsterClass` or `Character` instead of `MonsterClassv2` / `Characterv2` is `Medium`.

### Disabled Feature Flags
- Enabling `INCLUDE_NETWORK_SUPPORT` or `LAYERVIDEO_USE_OPENGL` without discussion is `High`.

### UI Files
- Any hand-edit of a `.ui` file is `High` (must go through Qt Designer only).
- Programmatic override of layout margins, spacing, stylesheets, or size policies set in `.ui` is `Medium`.

### Source Registration
- A new `.cpp`/`.h` file not added to `CMakeLists.txt` in the same change is `High` (silent link failure).

### Magic Numbers
- Non-trivial numeric or colour literals without a named constant are `Low` / `Medium` depending on scope.

## Security Checks (OWASP-Aligned)
- File path construction from user input — check for path traversal.
- XML parsing of untrusted campaign files — flag any direct use of attribute values without bounds checks.
