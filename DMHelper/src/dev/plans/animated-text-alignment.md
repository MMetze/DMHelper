---
feature_slug: animated-text-alignment
spec_path: DMHelper/src/dev/specs/animated-text-alignment.md
created: 2026-05-05
designer_model: opus
arch_review_required: true
arch_review_model: opus
arch_review_reason: Touches the OpenGL context boundary in PublishGLTextRenderer (recreateContent / setTextImage / updateProjectionMatrix interplay) and the rasterisation hand-off between a non-GL caller (EncounterTextEdit) and the GL paint path.
pre_impl_arch_review_requested: false
supersedes: null
status: in-progress
---

# Summary

Animated (scrolling) text encounters render at the wrong apparent font size and
with incorrect alignment when published, especially when a background layer is
present and at non-zero rotations. Root cause: `EncounterTextEdit` rasterises
the `QImage` in publish-window pixels at `windowWidth × textWidth%`, while
`PublishGLTextRenderer::recreateContent()` positions the texture in scene
units offset by `-(sceneWidth × textWidth%) / 2`; the two diverge whenever a
layer scene is present, and `getRotatedWidth()/getRotatedHeight()` skip the
rotation transpose in the layer-scene branch. Alignment is also baked into
the document at the narrowed band, so right/center alignment never reaches the
window edge.

The fix has three locked design decisions: (a) always rasterise at the full
rotated publish-window pixel size and apply `textWidth%` as left/right
margins on the root `QTextFrameFormat` so alignment is anchored to the full
window; (b) keep `QImage` sizing in window pixels and, when a layer scene is
present, scale the text texture's model matrix in `recreateContent()` by
`sceneWidth / windowWidth` so it covers the scene area without changing font
density; (c) make `setTextImage` preserve `_textPos` (with proportional
height-ratio scaling) and wire `TextEditFormatterFrame` signals so live edits
re-rasterise and update the player without scroll jump.

Subsystems touched: **UI shell** (`EncounterTextEdit`,
`TextEditFormatterFrame` wiring) and **publish/GL**
(`PublishGLTextRenderer`).

# Architectural Risk Assessment

- Trigger 1: **Hit** — modifies the GL-context boundary in
  `PublishGLTextRenderer` (`recreateContent`, `updateProjectionMatrix`,
  `setTextImage`); `setTextImage` is called from a non-GL context and must
  not perform GL calls.
- Trigger 2: **Not hit** — no `Layer` subclass added or modified.
- Trigger 3: **Not hit** — no serialization shape change.
- Trigger 4: **Not hit** — work spans UI shell and publish/GL only;
  publish/GL is not one of {battle, audio, campaign, UI shell} for this
  trigger's purposes (battle/audio/campaign are untouched).
- Trigger 5: **Not hit** — no new subsystem or feature flag.

`arch_review_required = true`, `arch_review_model = opus`,
`pre_impl_arch_review_requested = false`.

# Chunks

## Chunk 1: Renderer rotated-size transpose fix

- **id**: renderer-rotated-size-fix
- **summary**: Make `getRotatedWidth()`/`getRotatedHeight()` transpose for 90°/270° in the layer-scene branch, mirroring the no-layer branch.
- **dependencies**: []
- **branch**: agent/work/renderer-rotated-size-fix
- **files_to_modify**:
  - DMHelper/src/publishgltextrenderer.cpp — adjust `getRotatedWidth()` (~L385) and `getRotatedHeight()` (~L390) so the layer-scene branch returns transposed scene dimensions for 90°/270° rotations, matching the no-layer branch.
- **files_to_create**: []
- **integration_tasks**: []
- **acceptance_criteria**:
  - `getRotatedWidth()` returns scene height (not scene width) when a layer scene is present and rotation is 90° or 270°; `getRotatedHeight()` returns scene width in the same case.
  - The no-layer branch of both functions is unchanged.
  - Both functions remain `const` and have no new side effects (no `emit`, no member mutation).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - No magic numbers: rotation comparisons must use the same named constants/literals as the no-layer branch already uses (do not introduce new ones).
- **out_of_scope**:
  - Any change to `updateProjectionMatrix()` semantics.
  - Any change to `recreateContent()` (handled in chunk 4).

## Chunk 2: Renderer scene-size emission fix

- **id**: renderer-scene-size-emit-fix
- **summary**: Make `sceneSizeChanged` always emit publish-window/target size, not the layer scene size, so the rasteriser receives window pixels regardless of whether layers are present.
- **dependencies**: []
- **branch**: agent/work/renderer-scene-size-emit-fix
- **files_to_modify**:
  - DMHelper/src/publishgltextrenderer.cpp — in `updateSceneRect()` (the function that emits `sceneSizeChanged`) emit the publish-window/target size in both branches (with-layer and without-layer); remove the conditional that emits the layer scene size.
- **files_to_create**: []
- **integration_tasks**: []
- **acceptance_criteria**:
  - `sceneSizeChanged` is emitted with the publish-window/target size in both the layer-present and no-layer code paths.
  - No other emission of `sceneSizeChanged` anywhere in `publishgltextrenderer.cpp` carries the scene-rect size.
  - The internal `_scene` rect computation used by `updateProjectionMatrix()` is unchanged (only the emitted size is changed).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - The emit happens outside any `*GL*` function; this remains a non-GL signal emission and must not perform GL calls.
- **out_of_scope**:
  - Changes to `EncounterTextEdit::sceneRectUpdated()` (the receiver is already correct once the emitted size is window pixels).

## Chunk 3: Rasteriser uses full window width with margin-based text band

- **id**: rasteriser-full-window-with-margins
- **summary**: Rewrite `EncounterTextEdit::getDocumentTextImage()` (and `prepareImages()` if needed) so the `QImage` is always rasterised at the full rotated publish-window width, with `textWidth%` applied as left/right root-frame margins on the live `QTextDocument` (saved and restored around the rasterisation).
- **dependencies**: []
- **branch**: agent/work/rasteriser-full-window-with-margins
- **files_to_modify**:
  - DMHelper/src/encountertextedit.cpp — modify `getDocumentTextImage()` (~L578) to rasterise at `renderWidth` (the full rotated window width) instead of `renderWidth × textWidth% / 100`; before rasterisation, capture the current root `QTextFrameFormat`, set `leftMargin = rightMargin = renderWidth × (100 - textWidth%) / 200`, rasterise, then restore the original `QTextFrameFormat` (the live document must not be permanently mutated). Update `prepareImages()` (~L552) only if the call-site contract requires a different argument; the caller continues to pass the full window-pixel width.
  - DMHelper/src/encountertextedit.h — only if a helper signature must change to pass the full window width; do not add new public API beyond what is needed.
- **files_to_create**: []
- **integration_tasks**:
  - Add named `static constexpr` constants at the top of `encountertextedit.cpp` for the literal values used in the new margin formula (e.g. the `100` percent denominator and the `200` half-percent denominator), or reuse any existing constant if one already serves this purpose.
- **acceptance_criteria**:
  - `getDocumentTextImage()` calls the QImage constructor with width equal to the unmodified `renderWidth` argument (no `textWidth%` multiplication on the image dimensions).
  - The function captures the root `QTextFrameFormat` into a local before mutation and restores it before returning, so the live `QTextDocument` root frame format is identical before and after the call.
  - The new margin computation uses named `static constexpr` constants for any literal denominators, with no bare magic numbers.
  - The function does not emit `dirty()`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - No magic numbers — see integration task above.
  - No `dirty()` emission from this rasterisation path; `dirty()` is reserved for unsaved-data changes.
  - The function performs no GL calls (it is not a `*GL*` function).
- **out_of_scope**:
  - Any change to text colour, font selection, or other formatter behaviour.
  - Live-edit signal wiring (handled in chunk 6).

## Chunk 4: Renderer recreateContent positions in window pixels with scene scaling

- **id**: renderer-recreatecontent-window-pixels
- **summary**: In `PublishGLTextRenderer::recreateContent()` position the text texture using the publish-window pixel width (X offset = `-windowWidth / 2`, full window width spans the texture), and when a layer scene is present scale the text texture's model matrix by `sceneWidth / windowWidth` (and similarly for height) so it covers the full scene area without changing font density.
- **dependencies**: [renderer-rotated-size-fix]
- **branch**: agent/work/renderer-recreatecontent-window-pixels
- **files_to_modify**:
  - DMHelper/src/publishgltextrenderer.cpp — modify `recreateContent()` (~L401) to (a) compute the texture's X position from the rotated publish-window width rather than `getRotatedWidth() × textWidth% / 100`, and (b) when a layer scene is present and its rotated size differs from the rotated publish-window size, build the text-texture model matrix with a uniform-axis scale factor of `sceneWidth / windowWidth` (and `sceneHeight / windowHeight`) so the texture spans the scene area at the correct font density. Use `getRotatedWidth()`/`getRotatedHeight()` (now correct after chunk 1) for scene dimensions and a separate accessor or stored target size for the publish-window dimensions.
- **files_to_create**: []
- **integration_tasks**:
  - Add named `static constexpr` constants at the top of `publishgltextrenderer.cpp` for any literal scale or offset factors introduced (e.g. the `2.0f` half-width divisor) if they are not already named.
  - Document with a brief inline comment that `recreateContent()` is invoked from the GL paint path; any matrix setup added here is permitted GL-context work because the function name path is GL-active.
- **acceptance_criteria**:
  - `recreateContent()` no longer multiplies the texture X-offset by `textWidth% / 100`.
  - When the publish-window size and the scene size are equal, the resulting model matrix is identical (within named-constant precision) to the matrix produced before this change for the no-layer case.
  - When the scene size differs from the window size, the model matrix carries a `sceneWidth / windowWidth` X scale and `sceneHeight / windowHeight` Y scale on the text texture only.
  - No new bare magic numbers; any new scalars are named `static constexpr`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule: matrix and texture work in `recreateContent()` is permissible only because it is invoked from the GL paint path; do not introduce calls that assume a GL context outside this function.
  - No magic numbers.
- **out_of_scope**:
  - Changes to `setTextImage()` (handled in chunk 5).
  - Changes to `updateProjectionMatrix()` semantics other than what is needed to consume the new model-matrix scaling consistently.

## Chunk 5: setTextImage preserves scroll position

- **id**: renderer-settextimage-preserve-pos
- **summary**: Modify `PublishGLTextRenderer::setTextImage()` to preserve `_textPos` across image swaps; when the new image height differs from the previous height, scale `_textPos` by `newHeight / oldHeight` so the same visual line remains centred.
- **dependencies**: []
- **branch**: agent/work/renderer-settextimage-preserve-pos
- **files_to_modify**:
  - DMHelper/src/publishgltextrenderer.cpp — modify `setTextImage()` (~L228) to capture the previous text-image height before assignment; if both old and new heights are positive and differ, set `_textPos = _textPos × (newHeight / oldHeight)`; otherwise leave `_textPos` unchanged. Ensure the function still triggers whatever existing post-update path exists (e.g. content rebuild flag) — but do not perform GL calls here.
- **files_to_create**: []
- **integration_tasks**: []
- **acceptance_criteria**:
  - `setTextImage()` no longer assigns a constant value (e.g. `0.0f`) to `_textPos`.
  - The height-scaling branch is guarded against division by zero (only applied when previous image height > 0).
  - `setTextImage()` performs no GL calls (no `glXxx`, no `makeCurrent()`, no shader/buffer creation).
  - The function does not emit `dirty()`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule: `setTextImage()` is not a `*GL*` function; any GL state work must be deferred to the next paint via existing flags.
- **out_of_scope**:
  - Changes to `recreateContent()` (chunk 4).
  - Adding new public API.

## Chunk 6: Live-edit formatter wiring

- **id**: live-edit-formatter-wiring
- **summary**: Connect `TextEditFormatterFrame` formatter signals (font family, font size, alignment, colour, bold, italic, underline) so that, while `_isPublishing` is true, each change re-rasterises via `prepareImages()` and pushes the new image with `_renderer->setTextImage(_textImage)`.
- **dependencies**: [rasteriser-full-window-with-margins, renderer-settextimage-preserve-pos]
- **branch**: agent/work/live-edit-formatter-wiring
- **files_to_modify**:
  - DMHelper/src/encountertextedit.cpp — in the place where the `TextEditFormatterFrame` is constructed/owned, connect each existing formatter signal (`fontFamilyChanged`, `fontSizeChanged`, `fontBoldChanged`, `fontItalicsChanged`, `fontUnderlineChanged`, `alignmentChanged`, `colorChanged`) to a single private slot that, only when `_isPublishing` is true and `_renderer` is non-null, calls `prepareImages()` then `_renderer->setTextImage(_textImage)`.
  - DMHelper/src/encountertextedit.h — declare the new private slot (e.g. `void onFormatterChanged()`) used by the connections above.
- **files_to_create**: []
- **integration_tasks**:
  - The new slot must guard on `_isPublishing` and on `_renderer` being non-null before doing any work; when not publishing it is a no-op.
  - Use `Qt::AutoConnection` (the default) — both signal source and slot receiver live on the GUI thread.
  - The slot must not emit `dirty()`; visual-only re-rasterisation uses `changed()` if any signal is needed (none required here because the renderer is updated directly).
- **acceptance_criteria**:
  - All seven formatter signals listed above have explicit `connect()` calls to the new slot in `encountertextedit.cpp`.
  - The new slot is declared in the `private slots:` section of `encountertextedit.h`.
  - The slot's body short-circuits when `!_isPublishing || !_renderer`.
  - No `dirty()` emission in the new slot.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule: the slot calls `setTextImage()` (non-GL) only; no GL calls in the slot.
  - Dirty/changed signal rule: visual-only update — do not emit `dirty()`.
- **out_of_scope**:
  - Changes to `TextEditFormatterFrame` itself (signals already exist).
  - Changes to scroll-speed, animated toggle, or `textWidth` slider wiring (those already have their own paths).

# Cycle Log

## Chunk: renderer-rotated-size-fix

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:10:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: Fixed getRotatedWidth() and getRotatedHeight() layer-scene branch to use _encounter->getLayerScene().sceneSize() with rotation-transpose mirroring the no-layer branch. Bug was that the else branch still referenced _scene.getSceneRect() making both branches identical. No new files, no CMakeLists.txt changes.
- review_verdict: Pass
- review_findings:
  - Info: const qualifier absent from both function declarations in the header — pre-existing, outside this chunk's scope.
- next_action: merge

## Chunk: renderer-scene-size-emit-fix

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:11:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: Single-line change in updateSceneRect() — layer-scene branch now emits sceneSizeChanged(_targetSize) instead of the layer scene size. _scene.deriveSceneRectFromSize() call and all other logic unchanged. Consumers receive window-pixel dimensions in all cases; projection/scissor geometry unaffected.
- review_verdict: Pass
- review_findings: []
- next_action: merge

## Chunk: rasteriser-full-window-with-margins

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:12:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: getDocumentTextImage() now rasterises at full renderWidth with textWidth% applied as left/right QTextFrameFormat root-frame margins. Added #include <QTextFrame>, static constexpr FULL_PERCENT=100 and HALF_PERCENT_DIVISOR=200.0, removed unused absoluteWidth. Root frame format saved/restored in all code paths. No header changes.
- review_verdict: Pass
- review_findings: []
- next_action: merge

## Chunk: renderer-recreatecontent-window-pixels

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:13:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c

## Chunk: renderer-settextimage-preserve-pos

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:13:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c

# Architecture Review

# Escalations
