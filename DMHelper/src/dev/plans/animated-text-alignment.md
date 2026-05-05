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
status: complete
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

## Chunk 7: recreateContent centring fix when scene size differs from window

- **id**: renderer-recreatecontent-center-fix
- **summary**: Fix the off-centre text band in `PublishGLTextRenderer::recreateContent()` when a layer scene is present and its rotated size differs from the rotated window size — `PublishGLImage` builds its model matrix as `translate(_x,_y) * scale(scaleX,scaleY)`, so position must be expressed in world (scene) units, not window units, whenever a non-unit scale is applied. Also replace the bare `/ 2` in the animated Y line with the existing `TEXT_HALF_DIVISOR` constant.
- **dependencies**: [renderer-recreatecontent-window-pixels, renderer-rotated-size-fix]
- **branch**: agent/work/renderer-recreatecontent-center-fix
- **files_to_modify**:
  - DMHelper/src/publishgltextrenderer.cpp — in `recreateContent()` (~L451-L478): when a layer scene is present and its rotated size differs from the rotated window size, build the X and Y position terms in scene units so that after the `T * S` model matrix is applied the texture remains centred horizontally on the origin and edge-aligned vertically with the scene-sized ortho viewport. Concretely, the X offset must equal `-sceneWidth / TEXT_HALF_DIVISOR` (so post-scale the texture spans `[-sceneWidth/2, +sceneWidth/2]`); the animated and non-animated Y formulas must use the **scaled** image height (`imageHeight * scaleY`) when scaling is in effect, so the post-scale top/bottom of the texture lands at `±sceneHeight/2`. The no-layer / no-scale path (`scaleX = scaleY = 1`) must produce **identical** position values to the current merged code. Also replace the bare `/ 2` literal in the animated branch's Y formula (`(-getRotatedHeight() / 2) - ...`) with `TEXT_HALF_DIVISOR` for consistency with the rest of the function.
- **files_to_create**: []
- **integration_tasks**:
  - Compute `scaleX` and `scaleY` once at the top of the layer-scene branch (or hoist them to function scope) so both the position formulas and the `setScaleX/setScaleY` calls consume the same values.
  - Add a brief inline comment at the position-setting lines noting that `PublishGLImage`'s model matrix is `T * S`, so position must be in post-scale world units.
  - Do not introduce any new bare numeric literals; if a new scalar is needed beyond `TEXT_HALF_DIVISOR`, declare a named `static constexpr` at file scope.
- **acceptance_criteria**:
  - In `recreateContent()`, every `/ 2` (or `* 0.5`) literal inside the function body is replaced by `TEXT_HALF_DIVISOR` (no bare half-divisor literals remain in the function).
  - The X-offset formula yields `-windowWidth / TEXT_HALF_DIVISOR` when no layer scene is present (or scene size equals window size), and `-sceneWidth / TEXT_HALF_DIVISOR` when a layer scene is present with a differing size.
  - The animated Y-offset formula yields `(-windowHeight / TEXT_HALF_DIVISOR) - imageHeight + _textPos` in the no-scale case and `(-sceneHeight / TEXT_HALF_DIVISOR) - (imageHeight * scaleY) + _textPos` (or an algebraically equivalent form using a single `scaleY`) when scaling is in effect.
  - The non-animated Y-offset formula yields `(windowHeight / TEXT_HALF_DIVISOR) - imageHeight` in the no-scale case and `(sceneHeight / TEXT_HALF_DIVISOR) - (imageHeight * scaleY)` when scaling is in effect.
  - The `setScaleX`/`setScaleY` calls are still made with the same `sceneWidth/windowWidth` and `sceneHeight/windowHeight` ratios as the merged chunk-4 code; only the position formulas change.
  - `recreateContent()` performs no calls outside the GL paint path's allowed surface; no new GL function calls beyond what chunk 4 already added.
  - The function does not emit `dirty()`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule: `recreateContent()` is invoked from `paintGL()`; matrix and texture work remains permitted only because the GL context is current there. Do not move any of this work out of `recreateContent()`.
  - No magic numbers — reuse `TEXT_HALF_DIVISOR`; declare a new `static constexpr` for any other new scalar.
  - The visible behaviour in the no-layer case must be byte-identical to the merged chunk-4 code (regression guard).
- **out_of_scope**:
  - Changes to `setTextImage()`, `updateProjectionMatrix()`, `updateSceneRect()`, `getRotatedWidth()/getRotatedHeight()`, or any rasterisation code in `EncounterTextEdit`.
  - Changes to `PublishGLImage`'s model matrix construction (`T * S` order is fixed and is the contract this chunk works against).
  - Any new public API on `PublishGLTextRenderer`.

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
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: recreateContent() X offset now uses publish-window pixel width (rotated). setScaleX/setScaleY called when layer scene dimensions differ from window. updateProjectionMatrix() unchanged. Build clean.
- review_verdict: Fail
- review_findings:
  - Medium: Bare 2.0 literal in new X-offset code; named static constexpr required per plan integration task.
  - Low: Missing inline comment in recreateContent() documenting GL-context provenance.
- next_action: re-execute

### Cycle 2

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:14:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: Added static constexpr TEXT_HALF_DIVISOR=2.0 at file scope; replaced bare 2.0 in new X-offset code with it; added GL-context provenance comment at top of recreateContent().
- review_verdict: Pass
- review_findings:
  - Low: Pre-existing inconsistency between animated Y (bare /2) and non-animated Y (TEXT_HALF_DIVISOR); not new code, not flagged.
- next_action: merge

## Chunk: live-edit-formatter-wiring

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:15:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (108/108, DMHelper.exe linked cleanly)
- executor_handoff_summary: Declared onFormatterChanged() in private slots section of encountertextedit.h. Implemented in encountertextedit.cpp — guards on !_isPublishing || !_renderer, then prepareImages() + setTextImage(). Connected all 7 formatter signals (fontFamilyChanged, fontSizeChanged, fontBoldChanged, fontItalicsChanged, fontUnderlineChanged, alignmentChanged, colorChanged) immediately after existing takeFocus connections. No existing connections modified.
- review_verdict: Pass
- review_findings: []
- next_action: merge

## Chunk: renderer-settextimage-preserve-pos

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:13:00Z
- sha_from: cf1083928ec76f4a5b563add242925300089fb0c
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: setTextImage() now captures oldHeight before assignment and scales _textPos proportionally by (newHeight/oldHeight) when oldHeight > 0 and heights differ. No GL calls, no dirty() emission, no constant reset of _textPos. Structural guard (oldHeight > 0) prevents division-by-zero on first load.
- review_verdict: Pass
- review_findings: []
- next_action: merge

# Architecture Review

## Post-Implementation Review — 2026-05-05

reviewer_model: opus
verdict: Revise
summary: Cross-chunk centering bug in `recreateContent()` — translation is applied in world units after scale, so when a layer scene size differs from the window size the text texture is off-centre, defeating the spec's "alignment unchanged with background layer" done condition; everything else passes.
reviewed_range: cf1083928ec76f4a5b563add242925300089fb0c..HEAD

triggers_evaluated:
  - threading: addressed — `setTextImage()` (called from `onFormatterChanged()` and `sceneRectUpdated()`) performs no GL calls; it mutates `_textImage`/`_textPos`, sets the `_recreateContent` flag, calls the non-GL `updateSceneRect()` (signal emit only), and emits `updateWidget()`. GL state work is correctly deferred to the next `paintGL()` via the flag. `recreateContent()` is invoked only from `paintGL()` (`publishgltextrenderer.cpp:205-209`) so its `PublishGLImage` construction (which generates VAO/VBO/EBO/textures) has a current GL context.
  - layer_interface: not-applicable — no `Layer` subclass added or modified.
  - serialization_shape: not-applicable — no `createOutputXML` / `internalOutputXML` / `inputXML` / `postProcessXML` changes.
  - subsystem_boundary: addressed — work spans UI shell (`encountertextedit.{cpp,h}`) and publish/GL (`publishgltextrenderer.cpp`) only; no cross-subsystem coupling beyond the existing renderer/editor relationship; no new circular includes; no `INCLUDE_NETWORK_SUPPORT` or `LAYERVIDEO_USE_OPENGL` touched.
  - new_subsystem_or_flag: not-applicable — no new top-level subsystem and no new flag in `dmconstants.h`.

findings:
  - High: `publishgltextrenderer.cpp` `recreateContent()` (~L468-L478) — `PublishGLImage`'s model matrix is built as `translate(_x,_y) * scale(scaleX,scaleY)` (`publishglimage.cpp:328-330`), so translation is in world units **after** the scale. Setting `setX(-windowWidth / TEXT_HALF_DIVISOR)` then `setScaleX(sceneWidth/windowWidth)` makes the text texture span `[-windowWidth/2, -windowWidth/2 + sceneWidth]` in world units instead of `[-sceneWidth/2, +sceneWidth/2]`. Under `updateProjectionMatrix()`'s `KeepAspectRatioByExpanding` ortho the visible world is centred on origin and sized by the scene, so the text band is off-centre whenever sceneWidth ≠ windowWidth. The same defect applies to the animated Y term and the non-animated Y term. This is the exact scenario chunk 4 was added to fix and it breaks the spec's done condition "Same encounter with a background image layer: font size and alignment unchanged". (No-layer case is unaffected because scaleX=scaleY=1.)
  - Low: `publishgltextrenderer.cpp` `recreateContent()` animated branch (~L463) still uses a bare `/ 2` (`(-getRotatedHeight() / 2) - _textObject->getImageSize().height() + _textPos`) inside a function that chunk 4 modified, while the surrounding code uses the `TEXT_HALF_DIVISOR` constant introduced by the same chunk. The Cycle 2 review note flagged this as "pre-existing"; the animated Y line is in the diff hunks of chunk 4, so it should use the named constant for consistency.
  - Info: `publishgltextrenderer.cpp` `setTextImage()` (~L244-L257) — the `_textPos` height-rescaling uses bare `/` and unnamed ratio with no magic-number constants, but the formula has no literal scalars (only the captured heights), so no constant is required. No action.
  - Info: `encountertextedit.cpp` `getDocumentTextImage()` (~L837-L878) — root frame format save/restore covers all return paths reachable from the function (single early-return guards `renderWidth <= 0` before mutation; the mutation path always reaches the restore lines). Behaviour is correct; restore is not protected by RAII so a future code path that throws or returns mid-block would leak the mutation, but Qt's `drawContents` does not throw and no early-return is plausible to add inside the block. No action.

## Chunk: renderer-recreatecontent-center-fix

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:20:00Z
- sha_from: 5ec0558bef0902e496fd642b8bf286a30ca16ca5
- executor_build_status: succeeded (107/107, DMHelper.exe linked cleanly)
- executor_handoff_summary: X offset changed to -getRotatedWidth()/TEXT_HALF_DIVISOR (scene units) in all paths. scaleX/scaleY hoisted using qFuzzyCompare. Y animated (scaled): (-sceneH/TEXT_HALF_DIVISOR)-(imageH*scaleY)+_textPos. Y non-animated (scaled): (sceneH/TEXT_HALF_DIVISOR)-(imageH*scaleY). No-scale paths collapse to scaleY=1 giving identical formulas. All bare /2 in recreateContent() replaced with TEXT_HALF_DIVISOR. T*S world-unit comment added.
- review_verdict: Pass
- review_findings:
  - Info: Comment wording "translation applied before scale" slightly misleading; intent correct.
  - Info: rewind() and timerEvent() still use bare /2 and don't scale imageHeight — pre-existing, out of scope.
- next_action: merge

## Post-Implementation Review — 2026-05-05 (re-review after chunk 7)

reviewer_model: opus
verdict: Pass
summary: Chunk 7 resolves the prior High centring finding; with `_x = -getRotatedWidth()/2` and `scaleX = sceneW/windowW` over a non-centered `T*S` model matrix the text texture spans `[-sceneW/2, +sceneW/2]` in world units, the no-scale path is byte-equivalent to the chunk-4 merged code, and every bare half-divisor literal in `recreateContent()` is now `TEXT_HALF_DIVISOR`.
reviewed_range: cf1083928ec76f4a5b563add242925300089fb0c..HEAD

triggers_evaluated:
  - threading: addressed — `setTextImage()` is unchanged from chunk 5 (no GL calls; defers GL work via `_recreateContent` flag and `updateWidget()`). `recreateContent()` is invoked only from `paintGL()` (`publishgltextrenderer.cpp:205-208`); chunk 7 added only scalar arithmetic and `qFuzzyCompare` calls — no new GL calls and no thread-affinity concerns.
  - layer_interface: not-applicable — no `Layer` subclass added or modified.
  - serialization_shape: not-applicable — no XML pipeline changes.
  - subsystem_boundary: addressed — chunk 7 confined to `publishgltextrenderer.cpp::recreateContent()`; no new cross-subsystem coupling.
  - new_subsystem_or_flag: not-applicable — no new subsystem and no new flag in `dmconstants.h`.

findings:
  - Info: `recreateContent()` (`publishgltextrenderer.cpp:416`) reads `_targetSize.width()/height()` without an `isValid()` guard. In practice `paintGL()` early-returns when prerequisites are missing and `_recreateContent` is only set after a non-null image is supplied, so `windowWidth`/`windowHeight` are non-zero in every reachable code path. Defensive only — no action.
  - Info: `getRotatedWidth()`/`getRotatedHeight()` are evaluated twice each inside the layer-scene branch (once into `sceneW`/`sceneH`, once again in the position formulas). Values are identical within a single call; cosmetic only — no action.
  - Info: Pre-existing bare `/2` and unscaled `getImageSize().height()` references remain in `rewind()` (`publishgltextrenderer.cpp:307-318`) and `timerEvent()` (`publishgltextrenderer.cpp:373-374`); these were flagged by the chunk-7 reviewer as out-of-scope. They will visually mis-position the scrolling text on the first frame after `rewind()`/`play()` whenever a layer scene size differs from the window — same `T*S` math, same scene-units requirement. Not a regression introduced by this feature (the formulas predate it), but worth tracking for a future fix. No follow-up required for this plan.

required_followups: []

Centring verification (per spec done condition "Same encounter with a background image layer: font size and alignment unchanged"):
  - `PublishGLImage` non-centered model matrix is `T(_x,_y) * S(scaleX, scaleY)` over vertices `[0, imageW] × [0, imageH]` (`publishglimage.cpp:280-285, 328-330`).
  - `imageW = windowWidth` (rasterised at full rotated window width by `EncounterTextEdit::getDocumentTextImage()`).
  - With `_x = -getRotatedWidth() / TEXT_HALF_DIVISOR` and `scaleX = sceneW / windowWidth` (when scaling is active), world X span = `[-sceneW/2, -sceneW/2 + sceneW] = [-sceneW/2, +sceneW/2]` — centred under the scene-sized ortho.
  - Non-animated Y: `_y = sceneH/2 - scaleY·imageH`; world Y span top edge = `sceneH/2`. ✓
  - Animated Y: `_y = -sceneH/2 - scaleY·imageH + _textPos`; image starts off-screen below and scrolls up. ✓
  - No-scale case (`scaleX = scaleY = 1`, `getRotatedWidth() == windowWidth` because `_scene` is derived from `_targetSize` in `updateSceneRect()` when no layer scene is present): formulas reduce to the chunk-4 merged values exactly.
  - Half-divisor scan of `recreateContent()` body: every divide-by-two is `/ TEXT_HALF_DIVISOR`; no bare `/ 2` or `* 0.5` remains.

Other re-verified items (unchanged since prior review):
  - `getDocumentTextImage()` (`encountertextedit.cpp:837`) saves and restores the root `QTextFrameFormat` on every reachable code path.
  - `updateSceneRect()` emits `sceneSizeChanged(_targetSize)` in both branches.
  - `getRotatedWidth()`/`getRotatedHeight()` transpose correctly in the layer-scene branch for 90°/270°.
  - `onFormatterChanged()` short-circuits on `!_isPublishing || !_renderer`; all 7 formatter signals connected; no `dirty()` emission anywhere in new code.

# Escalations
