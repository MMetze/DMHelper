---
feature_slug: fow-edit-pipeline-perf
spec_path: DMHelper/src/dev/investigations/fow-edit-pipeline-perf.md
created: 2026-05-17
designer_model: opus
arch_review_required: true
arch_review_model: opus
arch_review_reason: Touches OpenGL upload pipeline (texture lifetime, storage format, glTexSubImage2D) and modifies the `LayerFow` Layer subclass across four chunks, including a structural pivot to a new `FowGraphicsItem`, a new `fowRegionChanged` signal contract, and a deferred-upload / deferred-destruction contract for GL state mutation that must never run from GUI-thread paint methods.
pre_impl_arch_review_requested: true
supersedes: DMHelper/src/dev/plans/fow-edit-pipeline-perf.md
status: in-progress
---

# Summary

The FOW interactive edit hot path currently does O(map_pixels) of work per
mouse-move sample: full-image QPixmap rebuild, GL object teardown/recreate,
two whole-image deep copies, full `glTexImage2D` + `glGenerateMipmap`,
viewport-wide scene repaint, and a per-sample `dirty()` cascade into the
campaign model. The investigation in
[fow-edit-pipeline-perf.md](dev/investigations/fow-edit-pipeline-perf.md)
documents eight findings (F1–F8) and prescribes a six-step fix sequence,
smallest-blast-radius first.

This plan implements that sequence in four chunks executed sequentially on
`agent/work/*` branches. Chunk 1 collapses investigation steps 1–2 (F4 skip
unused mipmap + F2 stop destroying the GL object). Chunk 2 collapses steps
3–4 (F6 suppress per-sample `dirty()` + F8 add a 16 ms coalescing timer with
a stroke-end flush hook). Chunk 3 is the structural pivot from steps 5
(F5 + F1 + F7): introduce `FowGraphicsItem` (a custom `QGraphicsItem` that
owns the FOW image by pointer and supports rect-scoped repaint), add a
`fowRegionChanged(QRect)` signal on `LayerFow`, and route the coalescer
through it. Chunk 4 is step 6 (F3): switch `_imageFow` to
`Format_RGBA8888`, move the vertical flip into the vertex layout, and add a
`PublishGLBattleBackground::updateImageRegion` path that uses
`glPixelStorei(GL_UNPACK_ROW_LENGTH, ...)` + `glTexSubImage2D` to upload
only the dirty stripe.

Subsystems touched: battle (LayerFow, FowGraphicsItem, BattleFrameMapDrawer,
MapFrame) and the player-window GL upload path
(`PublishGLBattleBackground`). No serialization shape change. No `.ui`
changes. No `.qrc` changes.
# Replanning Rationale

This plan supersedes the 2026-05-16 draft of the same slug after a
pre-impl Architecture Review (verdict `Revise`). The amended spec
(`fow-edit-pipeline-perf.md`, revised 2026-05-16, expanded F2)
identifies a single root cause behind every Critical finding in the
prior review: **`LayerFow::updateFowInternal()` and all FOW paint
methods execute on the GUI thread with no current OpenGL context.**
The prior plan inherited the investigation's earlier (incorrect)
parenthetical that placed `delete _fowGLObject` on the GL thread, and
encoded `_fowGLObject->updateImage(...)` / `updateImageRegion(...)`
calls directly inside GUI-thread dispatch paths. Those calls would
silently no-op via the existing `currentContext()` guards in
`PublishGLObject::cleanup()` and `PublishGLBattleBackground::loadTexture()`,
breaking the player-window FOW update.

What this plan does differently:

- **Universal deferred-upload / deferred-destruction contract.** No
  code reachable from a paint method, signal handler, undo replay,
  coalescer timer slot, or any other GUI-thread call site touches GL.
  Every GL-state mutation (upload, sub-upload, cleanup) is consumed
  from inside a `*GL*` function — specifically `playerGLPaint()` or
  `playerGLUninitialize()` — where the player window's GL context is
  current. `LayerFow` carries new pending-state members
  (`bool _fowGLImageDirty`, `QRect _fowGLPendingRegion`, and a
  `bool _fowGLDeferredDestroy` flag) that GUI-thread paths set; the
  next `playerGLPaint()` reads and clears them.
- **Player-window wake-up is signal-driven, not GL-driven.** The
  coalescer timer slot emits `changed()` (Layer's existing
  visual-only repaint signal) to wake the player renderer; it does
  not call into `_fowGLObject`.
- **Chunk 1's "stop deleting" is reframed as "defer the upload".**
  Chunk 1 introduces the pending-upload flag and the
  `playerGLPaint()` consumer; the destroy-per-stroke pathology is
  fixed by virtue of the deferred-upload, not by an inline
  `updateImage` substitution.
- **Resize teardown also deferred.** The remaining
  `delete _fowGLObject` in `applySize()` (the only remaining
  legitimate destroy site) is converted to a deferred-destroy flag
  consumed in `playerGLPaint()` / `playerGLUninitialize()`. Per the
  amended F2 sub-finding, the destroy path leaks GPU handles when no
  context is current; the deferred-destruction pattern is now both a
  correctness fix and a structural prerequisite for chunk 4.
- **Chunk 4 keeps `loadTexture` source-format-tolerant.** Rather than
  break every caller of `PublishGLBattleBackground` by removing the
  `convertToFormat(RGBA8888)` and `flipped(Qt::Vertical)` calls
  outright, the revision keeps them as **guarded fast-paths** (no-op
  when the source image is already `Format_RGBA8888`) so existing
  battle-background, layer-draw, layer-image, layer-video, and
  overlay-timer callers continue to render correctly. Only the FOW
  caller benefits from the zero-copy path. A pre-merge integration
  task enumerates every caller and confirms each one's source-image
  format/orientation; the V-coordinate vertex change moves behind a
  per-instance flag rather than a global flip.

Chunks reused: all four chunk identities (`gl-upload-reuse`,
`coalesce-and-defer-dirty`, `fow-graphics-item`, `subregion-upload`)
and their dependency order are preserved. Bodies are rewritten where
the prior wording placed GL calls on GUI-thread paths. No prior plan
chunks are dropped; no new chunks are added.

No `Cycle Log` failure modes to avoid (no chunks had executed yet
under the prior plan).
# Architectural Risk Assessment

- **Trigger 1 (threading / GL context boundary): Hit.** Chunks 1, 3, and 4
  modify the OpenGL upload pipeline. Chunk 4 introduces a new texture-storage
  format and `glTexSubImage2D` path using `glPixelStorei`. All GL calls must
  remain inside `playerGL*` functions; the coalescer fires on the GUI thread
  and must marshal nothing — its effect on GL only happens later via the
  player window's normal `paintGL` cycle.
- **Trigger 2 (Layer subclass): Hit.** Every chunk modifies `LayerFow`. Chunk
  3 changes the DM-side graphics item from `QGraphicsPixmapItem` to a new
  `FowGraphicsItem` subclass of `QGraphicsItem`.
- **Trigger 3 (serialization shape change): Not hit.** XML format for
  `LayerFow`, `UndoFow*` is unchanged.
- **Trigger 4 (>1 of battle/audio/campaign/UI shell): Not hit.** Battle only.
- **Trigger 5 (new top-level subsystem or `dmconstants.h` feature flag):
  Not hit.** `FowGraphicsItem` is a single helper class scoped to the battle
  subsystem.

arch_review_required = true, arch_review_model = opus,
pre_impl_arch_review_requested = true.

**Central GL-context constraint (per amended spec, F2 sub-finding).**
The amended investigation
[fow-edit-pipeline-perf.md](dev/investigations/fow-edit-pipeline-perf.md)
corrects a prior parenthetical that placed `delete _fowGLObject` on
the GL thread. In reality, `updateFowInternal()` and every FOW paint
method (`paintFoWPoint`, `paintFoWPoints`, `paintFoWRect`,
`paintFoWPolygon`, `fillFoW`, `applyPaintTo`, `editSettings`) execute
on the GUI thread with no current OpenGL context. The existing
destroy-on-edit pattern survives only because
`PublishGLObject::cleanup()` silently skips `glDeleteTextures` when
`QOpenGLContext::currentContext()` is null and the next
`playerGLInitialize()` recreates the object from scratch — at the
cost of orphaned GPU handles per stroke sample (bounded in practice
but a real leak per the spec amendment).

Every chunk in this plan therefore observes a single, common
contract: **no GL-state mutation — neither texture upload, sub-upload,
nor destruction — may originate from a GUI-thread call site.**
GUI-thread paths set pending-state members on `LayerFow`
(`_fowGLImageDirty`, `_fowGLPendingRegion`, `_fowGLDeferredDestroy`);
`playerGLPaint()` consumes them. This is the deferred-upload /
deferred-destruction pattern referenced throughout the chunks below
and is the structural reason chunk 1 reframes the original
"stop deleting, call updateImage instead" idea into a flag-and-consume
design.

# Chunks

## Chunk 1: GL upload reuse and mipmap gating

- **id**: gl-upload-reuse
- **summary**: Stop destroying `_fowGLObject` per stroke sample. Introduce a `bool _fowGLImageDirty` flag on `LayerFow` set by `updateFowInternal()` (GUI thread, no GL context). The actual `_fowGLObject->updateImage(getImage())` call moves into `playerGLPaint()`, where the player window's GL context is current, and is consumed-and-cleared from the flag. Also convert the remaining resize-time `delete _fowGLObject` in `applySize()` into a deferred-destroy flag (`bool _fowGLDeferredDestroy`) consumed by the same `playerGLPaint()` consumer (and by `playerGLUninitialize()`). Separately: in `PublishGLBattleBackground::loadTexture()`, gate `glGenerateMipmap` on `_textureParam` being a mipmap-capable filter.
- **dependencies**: []
- **branch**: agent/work/gl-upload-reuse
- **files_to_modify**:
  - layerfow.h — add private `bool _fowGLImageDirty = false;` and `bool _fowGLDeferredDestroy = false;` members. Add a private `void consumePendingGLUpdates()` helper declaration documented as "callable only from a `*GL*` function with a current GL context".
  - layerfow.cpp — in `updateFowInternal()`, remove the `delete _fowGLObject; _fowGLObject = nullptr;` block; replace with `_fowGLImageDirty = true;` (no GL calls). In `applySize()`, replace the `delete _fowGLObject; _fowGLObject = nullptr;` block with `if(_fowGLObject) _fowGLDeferredDestroy = true;`. Implement `consumePendingGLUpdates()`: if `_fowGLDeferredDestroy`, `delete _fowGLObject; _fowGLObject = nullptr; _fowGLDeferredDestroy = false; _fowGLImageDirty = false;` (the next `playerGLInitialize` will rebuild). Else if `_fowGLImageDirty && _fowGLObject`, call `_fowGLObject->updateImage(getImage())` and clear the flag. Call `consumePendingGLUpdates()` as the first statement of `playerGLPaint()` (before the existing `_fowGLObject != nullptr` shader path) and as the first statement of `playerGLUninitialize()`. Name the mipmap-filter predicate locally (e.g. `static constexpr` helper at the top of the cpp) — no magic GL enums inline.
  - publishglbattlebackground.cpp — in `loadTexture()`, gate `glGenerateMipmap` on `_textureParam` being one of the mipmap-capable filters (`GL_NEAREST_MIPMAP_NEAREST`, `GL_NEAREST_MIPMAP_LINEAR`, `GL_LINEAR_MIPMAP_NEAREST`, `GL_LINEAR_MIPMAP_LINEAR`). Use a named `static constexpr` predicate or named constant set; do not inline magic GL enums.
- **files_to_create**: []
- **integration_tasks**:
  - Document inline in `consumePendingGLUpdates()` that this is the **only** function in `LayerFow` permitted to touch `_fowGLObject`'s GL-state-mutating methods (constructor, `updateImage`, `setImage`, destructor). Every other call site sets a pending flag instead. The chunk's reviewer should grep `_fowGLObject->` and `delete _fowGLObject` after the change to confirm no GUI-thread call site remains.
  - `applySize()` runs on the GUI thread; deferring its destroy means the previous GL object remains alive (and rendered) for up to one player-window paint cycle after a resize. This is acceptable because resize already invalidates the FOW image content and the next `playerGLPaint()` will rebuild via `playerGLInitialize` on the cycle after the destroy. Document this inline.
  - The mipmap-filter predicate in `publishglbattlebackground.cpp` must also be applied to any future `updateImage()` / `updateImageRegion()` paths (chunk 4 will reuse the predicate); name it accordingly so chunk 4 does not re-derive it.
- **acceptance_criteria**:
  - `LayerFow::updateFowInternal()` contains no `delete _fowGLObject`, no `_fowGLObject->updateImage(...)`, and no other call into `_fowGLObject`'s GL-mutating methods; its only `_fowGLObject`-related statement is `_fowGLImageDirty = true;`.
  - `LayerFow::applySize()` contains no `delete _fowGLObject`; instead sets `_fowGLDeferredDestroy = true` when `_fowGLObject` is non-null.
  - `LayerFow` declares a private `consumePendingGLUpdates()` helper.
  - `LayerFow::playerGLPaint()` calls `consumePendingGLUpdates()` as its first statement (before any other `_fowGLObject` access).
  - `LayerFow::playerGLUninitialize()` calls `consumePendingGLUpdates()` (or equivalently consumes both flags) before tearing down its own GL state.
  - `PublishGLBattleBackground::loadTexture()` only calls `glGenerateMipmap` when `_textureParam` is one of the mipmap-capable filters, expressed via a named helper/constant set — not a literal enum comparison.
  - A grep of `layerfow.cpp` for `_fowGLObject->` returns matches only inside `consumePendingGLUpdates()`, `playerGLInitialize()`, `playerGLPaint()`, `playerGLUninitialize()`, and `playerGLResize()`-equivalent GL helpers — never inside `updateFowInternal`, `applySize`, `paintFoW*`, `fillFoW`, `applyPaintTo`, `editSettings`, or any constructor/destructor.
  - Windows debug build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule (per amended spec, central constraint): no GL-mutating call from a GUI-thread function. `consumePendingGLUpdates()` is the sole consumer and is only invoked from `*GL*` functions.
  - No magic numbers — name the mipmap-filter set per cpp-qt.instructions.md.
  - `dirty()` semantics unchanged in this chunk.
- **out_of_scope**:
  - Any change to `_imageFow` storage format (chunk 4).
  - Coalescing or `dirty()` suppression (chunk 2).
  - Any new public method on `PublishGLBattleBackground` (chunk 4).
  - Partial-region GL upload (chunk 4); this chunk still calls `updateImage(getImage())` with the full image, just deferred.

## Chunk 2: Coalesce updates and defer `dirty()` to stroke end

- **id**: coalesce-and-defer-dirty
- **summary**: Stop emitting `dirty()` from per-sample `paintFoWPoint` /
  `paintFoWPoints`. Add a single-shot coalescing `QTimer` on `LayerFow` that
  defers `updateFowInternal()` work to once per ~16 ms window. Add a public
  `flushPendingUpdate()` that the mouse-up handlers invoke to dispatch
  synchronously at stroke end (along with the `dirty()` they already emit).
- **dependencies**: [gl-upload-reuse]
- **branch**: agent/work/coalesce-and-defer-dirty
- **files_to_modify**:
  - layerfow.h — add a `QTimer* _updateCoalescer` member (or
    `_updateCoalescerPending` flag with `QTimer::singleShot`), a
    `flushPendingUpdate()` public method, and a private
    `requestFowUpdate()` slot. Add a `static constexpr int
    FOW_UPDATE_COALESCE_MS = 16;` near the top of the .cpp.
  - layerfow.cpp — wire the timer in the constructor; replace the direct
    `updateFowInternal(); emit dirty();` calls in `paintFoWPoint` and
    `paintFoWPoints` with `requestFowUpdate()` only (no `dirty()` per
    sample). Keep direct `updateFowInternal(); emit dirty();` in
    `fillFoW`, `paintFoWRect`, `paintFoWPolygon`, `applyPaintTo`,
    `editSettings`. Implement `flushPendingUpdate()` to cancel the timer
    and call `updateFowInternal()` immediately when work is pending.
  - battleframemapdrawer.cpp — in `endPath()`, after the existing
    `_undoPath = nullptr;` / `emit dirty();` logic, call
    `layer->flushPendingUpdate()` on the active FOW layer before the
    `dirty()` emit so downstream listeners see consistent imagery.
  - mapframe.cpp — in the `QEvent::MouseButtonRelease` branch of the FOW
    edit-mode event filter (the block currently nulling `_undoPath` and
    emitting `dirty()`), call `flushPendingUpdate()` on the active FOW
    layer before emitting `dirty()`.
- **files_to_create**: []
- **integration_tasks**:
  - Connect `_updateCoalescer->timeout` to a private slot that calls `updateFowInternal()` and then emits `changed()` (Layer's existing visual-only signal) to wake the player renderer. The slot performs **no GL calls** — `updateFowInternal()` only sets `_fowGLImageDirty = true` (chunk-1 contract); the next `playerGLPaint()` consumes the flag. Do **not** emit `dirty()` from the timeout slot — `dirty()` semantics remain "unsaved data changed" and continue to fire only from the call sites listed above.
  - Document inline at the timer-slot definition: "Deferred-upload contract — this slot runs on the GUI thread with no current GL context. It MUST NOT call into `_fowGLObject`'s GL-mutating methods. The chunk-1 pending-flag pattern is the only permitted path to a GL upload."
  - `applyPaintTo()` (undo/redo replay) must continue to call
    `updateFowInternal()` directly at the end of its `_batchProcessing`
    block and must not go through the coalescer; document this in an
    inline comment.
- **acceptance_criteria**:
  - `paintFoWPoint` and `paintFoWPoints` no longer contain `emit dirty()`
    in either the batch or non-batch branch.
  - `paintFoWPoint` and `paintFoWPoints` route their non-batch update
    through `requestFowUpdate()`, not `updateFowInternal()`.
  - `fillFoW`, `paintFoWRect`, `paintFoWPolygon`, `applyPaintTo`,
    `editSettings` still call `updateFowInternal()` and still emit
    `dirty()` (where they did before).
  - `LayerFow` declares `flushPendingUpdate()` as a public method.
  - `BattleFrameMapDrawer::endPath()` and the MOUSE-up branch in
    `mapframe.cpp`'s FOW edit-mode event filter call
    `flushPendingUpdate()` on the active FOW layer.
  - The timer slot emits `changed()` to wake the player renderer; it does **not** call into `_fowGLObject` or any other GL-mutating API.
  - The 16 ms interval is expressed as a named constant, not a literal.
  - Windows debug build succeeds.
- **constraints_in_scope**:
  - `dirty()` vs `changed()` signal semantics — never emit `dirty()` from
    the timer slot; never emit `dirty()` from a constructor or
    `inputXML()`. The timer slot emits `changed()` only.
  - The coalescing timer runs on the GUI thread with no current GL context; the slot must not call any GL-mutating method on `_fowGLObject`. Chunk-1's pending-flag pattern is the sole upload path.
  - No magic numbers — the 16 ms interval is a named constant.
- **out_of_scope**:
  - Adding `fowRegionChanged` signal (chunk 3) — the coalescer in this
    chunk still triggers a full-image `updateFowInternal()`.
  - Changing the DM graphics item type (chunk 3).
  - Any GL-side change (chunk 4).

## Chunk 3: `FowGraphicsItem` + `fowRegionChanged` signal

- **id**: fow-graphics-item
- **summary**: Introduce `FowGraphicsItem` (a `QGraphicsItem` subclass that
  holds a pointer to `LayerFow`'s combined image and implements `paint()`
  with `painter->drawImage(option->exposedRect, image, option->exposedRect)`).
  Replace `LayerFow`'s `QGraphicsPixmapItem* _graphicsItem` with the new
  type. Add `fowRegionChanged(const QRect&)` signal on `LayerFow`. Make each
  paint method compute its footprint rect and have the coalescer accumulate
  rects into a union, dispatching once per timer fire via
  `update(unionRect)` on the DM item and via the chunk-1 deferred-upload
  flags (`_fowGLImageDirty` and a new `_fowGLPendingRegion` accumulator)
  for the GL side. The actual GL upload remains in `playerGLPaint()` per
  the chunk-1 contract; this chunk does not move GL calls onto the GUI
  thread. Keep `getImage()` available to the new item for the textured-FOW
  composite path.
- **dependencies**: [coalesce-and-defer-dirty]
- **branch**: agent/work/fow-graphics-item
- **files_to_modify**:
  - layerfow.h — change `_graphicsItem` type to `FowGraphicsItem*`; add
    `void fowRegionChanged(const QRect& region);` signal; add private
    `QRect _pendingDirtyRect;` accumulator for the DM-side dispatch; add
    private `QRect _fowGLPendingRegion;` accumulator consumed by
    chunk-1's `consumePendingGLUpdates()` (chunk-4 will read the region,
    chunk-1/3 only union into it); expand `requestFowUpdate()` to accept
    a `QRect` and union it into `_pendingDirtyRect`; change the timer slot
    to dispatch with `_pendingDirtyRect` via `dispatchFowUpdate(region)`.
  - layerfow.cpp — each of `paintFoWPoint`, `paintFoWPoints`,
    `paintFoWRect`, `paintFoWPolygon`, `fillFoW` computes a tight `QRect`
    (radius footprint for points, bounding rect for shape/polygon,
    full-image rect for fill) and passes it to `requestFowUpdate(rect)`
    (or direct dispatch for non-batched fill/rect/polygon). Refactor
    `updateFowInternal()` into a private `dispatchFowUpdate(const QRect& region)`
    that performs **only GUI-thread-safe work**: (a) emits
    `fowRegionChanged(region)` so the new DM-side item calls
    `update(region)`; (b) sets `_fowGLImageDirty = true` and unions
    `region` into a new private `QRect _fowGLPendingRegion` accumulator
    (chunk-4 will consume the region; this chunk still uploads full-image
    in `consumePendingGLUpdates()`, so the accumulator may be ignored by
    chunk-3's consumer but is populated for chunk-4); (c) emits
    `changed()` to wake the player renderer. `dispatchFowUpdate()` does
    **not** call into `_fowGLObject` — the chunk-1 deferred-upload
    contract is preserved. Construct a `FowGraphicsItem` in
    `dmInitialize()` instead of calling `scene->addPixmap(...)`. In
    `applySize()`, replace the previous `setPixmap` follow-up with item
    construction (or `prepareGeometryChange()` + `update()` if the item
    already exists) and keep the chunk-1 `_fowGLDeferredDestroy = true`
    line.
  - CMakeLists.txt — register `fowgraphicsitem.cpp` in the source list and
    `fowgraphicsitem.h` in the header list (alphabetical insertion next to
    the existing `layerfow.cpp` / `layerfow.h` entries).
- **files_to_create**:
  - fowgraphicsitem.h — declares `class FowGraphicsItem : public
    QGraphicsItem` with constructor taking a const QImage pointer (lifetime
    owned by `LayerFow`), `boundingRect()` override, `paint(QPainter*,
    const QStyleOptionGraphicsItem*, QWidget*)` override, and an
    `updateRegion(const QRect& region)` convenience that calls
    `QGraphicsItem::update(QRectF(region))` so callers don't have to
    convert.
  - fowgraphicsitem.cpp — implements the above. `paint()` reads
    `option->exposedRect`, intersects with the image bounds, and calls
    `painter->drawImage(exposedRect, *_image, exposedRect.toAlignedRect())`.
- **integration_tasks**:
  - `LayerFow::fowRegionChanged` is for **visual update only** — it is a
    `changed()`-like signal, not a `dirty()`-like signal. Document this in
    the header next to the signal declaration. `dirty()` continues to fire
    only from the chunk-2 call sites.
  - `FowGraphicsItem` reads `LayerFow`'s combined image. The simplest
    contract is to pass a `const QImage*` pointing at `_imageFow` and
    re-composite the texture on each `paint()` via a cached combined
    `QImage` member on `FowGraphicsItem` invalidated by `updateRegion`.
    Alternatively, expose a `LayerFow::getCombinedImage()` accessor returning
    a const reference. Choose whichever keeps `FowGraphicsItem` independent
    of `LayerFow`'s header (forward declaration only). Document the choice
    inline.
  - Set `QGraphicsItem::ItemUsesExtendedStyleOption` on the new item so
    `option->exposedRect` is populated. Do **not** set
    `DeviceCoordinateCache` on the FOW item itself — its contents mutate.
  - The coalescer's pending-rect accumulator (`_pendingDirtyRect`, DM-side)
    must reset to an empty `QRect()` after each timer dispatch, and a fill
    operation must replace (not union) the accumulator with the full-image
    rect. The GL-side accumulator (`_fowGLPendingRegion`) is cleared by
    `consumePendingGLUpdates()` inside `playerGLPaint()` (chunk-1 owns this
    contract); the chunk-3 dispatch path only writes to it.
  - Document inline at `dispatchFowUpdate()`: "Deferred-upload contract —
    runs on the GUI thread. Sets pending flags and emits signals; never
    calls into `_fowGLObject`. The next `playerGLPaint()` consumes
    `_fowGLImageDirty` / `_fowGLPendingRegion` via
    `consumePendingGLUpdates()`."
- **acceptance_criteria**:
  - `fowgraphicsitem.h` and `fowgraphicsitem.cpp` exist and are listed in
    `CMakeLists.txt`'s explicit source/header lists.
  - `FowGraphicsItem` inherits `QGraphicsItem`, overrides `boundingRect()`
    and `paint()`, and declares `updateRegion(const QRect&)`.
  - `LayerFow`'s `_graphicsItem` member type is `FowGraphicsItem*` (no
    longer `QGraphicsPixmapItem*`).
  - `LayerFow` declares signal
    `void fowRegionChanged(const QRect& region);`.
  - Each of `paintFoWPoint`, `paintFoWPoints`, `paintFoWRect`,
    `paintFoWPolygon`, `fillFoW` computes and propagates a rect to the
    dispatch path (no full-image fallback inside the rect calculation
    itself except for `fillFoW`).
  - `LayerFow::dmInitialize` constructs a `FowGraphicsItem` rather than
    calling `scene->addPixmap(...)`.
  - `FowGraphicsItem::paint()` uses `option->exposedRect` to scope its
    `drawImage` call.
  - Windows debug build succeeds.
- **constraints_in_scope**:
  - Never override `.ui` properties from code (n/a — no `.ui` touched).
  - `dirty()` vs `changed()`: `fowRegionChanged` is a visual-update
    signal, semantically `changed()`-like. The timer slot continues to
    emit `changed()` (chunk-2 contract) so the player window repaints and
    consumes pending GL state on its next `playerGLPaint()`.
  - GL context rule (chunk-1 contract, reaffirmed): the timer slot and
    `dispatchFowUpdate()` perform no GL calls. Pending state is consumed
    in `playerGLPaint()` via `consumePendingGLUpdates()`. Any new code in
    this chunk that needs to touch `_fowGLObject` must go through the
    chunk-1 helper, not inline.
- **out_of_scope**:
  - Switching `_imageFow` to `Format_RGBA8888` (chunk 4).
  - Removing the CPU vertical flip in `loadTexture` (chunk 4).
  - Adding `glTexSubImage2D` upload path (chunk 4).
  - Adding `DeviceCoordinateCache` to non-FOW graphics items — that work is
    deferred; this chunk leaves cache modes unchanged on other items.
  - Optimising `getImage()` itself — `dispatchFowUpdate()` still calls `getImage()` to produce the composite image, which remains an O(map_pixels) operation regardless of the dirty-rect scoping. Rect-scoping only reduces the *consumer* cost (DM `paint()` and chunk-4 GL upload). A future chunk could split the composite into per-region work, but it is not in scope here.

## Chunk 4: Storage-format change and subregion GL upload

- **id**: subregion-upload
- **summary**: Switch `_imageFow` and `_imageFowTexture` to `QImage::Format_RGBA8888` so the FOW-side `loadTexture` path can skip the CPU `convertToFormat` and `flipped` copies. Add `PublishGLBattleBackground::updateImageRegion(const QImage&, const QRect&)` that uses `glPixelStorei(GL_UNPACK_ROW_LENGTH, ...)` + `glTexSubImage2D` to upload only the dirty stripe. Extend chunk-1's `consumePendingGLUpdates()` to call `updateImageRegion(getImage(), _fowGLPendingRegion)` when a partial region is pending and `updateImage(getImage())` when the full-image region is pending — the GL call still happens inside `playerGLPaint()`, never on the GUI thread. Keep `loadTexture()` source-format-tolerant by reducing the `convertToFormat`/`flipped` work to a guarded fast-path (no-op when input is already `Format_RGBA8888` with a per-instance orientation flag), so non-FOW callers (`LayerImage`, `LayerVideo`, `LayerVideoEffect`, `LayerDraw`, `OverlayTimer`, battle background) continue to render correctly with their existing source images.
- **dependencies**: [fow-graphics-item]
- **branch**: agent/work/subregion-upload
- **files_to_modify**:
  - layerfow.h — add inline comment near `_imageFow` declaration documenting the storage-format change to `Format_RGBA8888` and noting that `QPainter` composition still works because Qt converts internally.
  - layerfow.cpp — change `QImage::Format_ARGB32_Premultiplied` to `QImage::Format_RGBA8888` at the two construction sites in `initialize()`. Verify the `QPainter` composition modes used by paint methods (`CompositionMode_Source`, `CompositionMode_DestinationIn`) still produce correct output against `Format_RGBA8888` (they do — Qt converts internally; document inline). Extend chunk-1's `consumePendingGLUpdates()` to inspect `_fowGLPendingRegion`: if equal to the full-image rect or empty, call `_fowGLObject->updateImage(getImage())`; else call `_fowGLObject->updateImageRegion(getImage(), _fowGLPendingRegion)`. Clear `_fowGLPendingRegion` to an empty rect after consumption. Construct `_fowGLObject` with a "source-is-RGBA8888, no-CPU-flip" flag so chunk-1's full-image upload path takes the zero-copy branch (see `publishglbattlebackground.h/.cpp` changes below).
  - publishglbattlebackground.h — declare `void updateImageRegion(const QImage& image, const QRect& region);`. Add a constructor overload (or a new setter `setSourceImageOptions(bool sourceIsRgba8888, bool sourceNeedsVerticalFlip)`) that records per-instance flags consumed by `loadTexture()` / `updateImage()` / `updateImageRegion()` / `createImageObjects()`. Default values must preserve existing behaviour: `sourceIsRgba8888 = false`, `sourceNeedsVerticalFlip = true` (CPU flip on).
  - publishglbattlebackground.cpp — implement `updateImageRegion`: guard with the existing `QOpenGLContext::currentContext()` null pattern from `loadTexture`; bind `_textureID`; set `glPixelStorei(GL_UNPACK_ROW_LENGTH, image.bytesPerLine() / BYTES_PER_PIXEL_RGBA)`; compute `flippedY = _imageSize.height() - region.y() - region.height()` and call `glTexSubImage2D(GL_TEXTURE_2D, 0, region.x(), flippedY, region.width(), region.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.constScanLine(region.y()) + region.x() * BYTES_PER_PIXEL_RGBA)`; restore `glPixelStorei(GL_UNPACK_ROW_LENGTH, 0)`; call `glGenerateMipmap` only via the chunk-1 mipmap-filter predicate. In `loadTexture()`, gate the `convertToFormat(RGBA8888)` and `flipped(Qt::Vertical)` calls on the new per-instance flags: when `sourceIsRgba8888 == true` skip `convertToFormat`; when `sourceNeedsVerticalFlip == false` skip the flip and instead emit V-inverted texcoords from `createImageObjects()` for this instance only. In `createImageObjects()`, branch the vertex-array V coordinates on the per-instance flip flag — do not change the global vertex layout. Name `BYTES_PER_PIXEL_RGBA = 4` as `static constexpr int`. Mark the V-coordinate values in the no-CPU-flip branch with an inline comment documenting GL bottom-left origin convention.
- **files_to_create**: []
- **integration_tasks**:
  - All GL calls in `updateImageRegion`, the new no-CPU-flip `loadTexture` branch, and the chunk-3 dispatch's downstream consumer execute inside `playerGLPaint()` via the chunk-1 `consumePendingGLUpdates()` helper — **never** from the GUI thread. `dispatchFowUpdate()` (chunk 3) only writes to `_fowGLImageDirty` and `_fowGLPendingRegion` and continues to emit `changed()` to wake the renderer. The chunk's reviewer must grep `_fowGLObject->updateImage` and `_fowGLObject->updateImageRegion` to confirm both appear only inside `consumePendingGLUpdates()`.
  - **Pre-merge integration task — enumerate every `PublishGLBattleBackground` caller and verify source-image format/orientation.** The known caller set, derived from a workspace grep of `new PublishGLBattleBackground(` and `->updateImage(` / `->setImage(`, is: `layerfow.cpp` (FOW — switches to `Format_RGBA8888`, no flip), `layerimage.cpp` (LayerImage — keeps `Format_ARGB32_Premultiplied` source, CPU flip on), `layervideo.cpp` (LayerVideo — VLC-decoded RGBA but Qt-side reshape; CPU flip on), `layervideoeffect.cpp` (same family), `layerdraw.cpp` (`GL_LINEAR`-filtered draw objects; CPU flip on), `overlaytimer.cpp` (timer overlay; CPU flip on), `mainwindow.cpp` (publish-image dispatcher path; CPU flip on). For each, confirm the source image's `format()` and required orientation by reading the call site in this chunk's Execution; record the audit results inline in the cpp constructor or in the chunk's handoff. Only the FOW caller passes `sourceIsRgba8888 = true, sourceNeedsVerticalFlip = false`; every other caller defaults to `false, true` and is unaffected by this chunk. If the audit finds any caller already passing RGBA8888 but still relying on the CPU flip, document it as a follow-up but **do not** change its behaviour in this chunk.
  - **macOS smoke test — human-mediated.** After Windows debug build passes, hand the merged chunk-4 branch to the human for a manual macOS build and a single-stroke FOW smoke test on a 4k map in both DM-only and DM+player-publishing modes (the spec amendment flags `Format_RGBA8888` interacting differently with the CoreGraphics-backed `QPainter` than `Format_ARGB32_Premultiplied`). Record the result in the chunk's `Cycle Log` handoff. Coordinator must surface this as a checkpoint before declaring the chunk merge-ready.
  - Constants `BYTES_PER_PIXEL_RGBA = 4`, the mipmap-filter predicate (reuse the chunk-1 helper), the per-instance flip flag, and the V-coordinate values in the no-CPU-flip branch must be named or accompanied by an inline GL-origin comment per cpp-qt.instructions.md.
- **acceptance_criteria**:
  - `LayerFow::initialize()` constructs `_imageFow` and `_imageFowTexture` with `QImage::Format_RGBA8888`.
  - `PublishGLBattleBackground::updateImageRegion()` exists, is declared in the header, guards itself with `QOpenGLContext::currentContext()`, and calls `glPixelStorei(GL_UNPACK_ROW_LENGTH, ...)` and `glTexSubImage2D` with `GL_UNPACK_ROW_LENGTH` restored to 0 after the upload.
  - `PublishGLBattleBackground::loadTexture()` retains its `convertToFormat` and `flipped`/`mirrored` calls **as guarded fast-paths** that are skipped only when the per-instance `sourceIsRgba8888` / `sourceNeedsVerticalFlip` flags say so. Default-constructed callers continue to take the convert+flip path.
  - The FOW `_fowGLObject` construction passes `sourceIsRgba8888 = true, sourceNeedsVerticalFlip = false`; all other `PublishGLBattleBackground` construction sites continue to use the defaults.
  - `createImageObjects()` branches the vertex-array V coordinates on the per-instance flip flag; the global vertex layout (used by all defaulting callers) is unchanged.
  - `LayerFow`'s `consumePendingGLUpdates()` routes partial-region uploads through `updateImageRegion()` and full-region uploads through `updateImage()`.
  - A grep of `layerfow.cpp` confirms `_fowGLObject->updateImage` and `_fowGLObject->updateImageRegion` appear **only** inside `consumePendingGLUpdates()`.
  - The chunk's handoff note enumerates each `PublishGLBattleBackground` caller and the per-instance flag values it uses.
  - All literal byte-count and pixel-format integers in new code are named constants; the V-coordinate constants in the no-CPU-flip vertex branch carry an inline comment documenting the GL bottom-left origin convention.
  - Windows debug build succeeds.
  - Handoff note records macOS smoke-test status (Pending until human confirms; the chunk is **not** merge-ready until the macOS result is `pass`).
- **constraints_in_scope**:
  - GL context rule (central constraint, per amended spec): `updateImageRegion()`, `updateImage()`, and the no-CPU-flip `loadTexture` branch are all invoked exclusively from `consumePendingGLUpdates()` inside `playerGLPaint()`. The internal `QOpenGLContext::currentContext()` null-guard remains as defence-in-depth.
  - Cross-platform Qt: `Format_RGBA8888` storage may behave differently under macOS CoreGraphics QPainter back-end; the macOS smoke test is the gate.
  - No magic numbers — `BYTES_PER_PIXEL_RGBA` and the mipmap-filter predicate must be named.
  - `glPixelStorei(GL_UNPACK_ROW_LENGTH, 0)` must be restored after the subregion upload to avoid bleeding state into other callers' subsequent `glTexImage2D` calls.
  - Backwards compatibility for non-FOW `PublishGLBattleBackground` callers is mandatory; the guarded fast-paths exist so existing callers continue to work without modification.
- **out_of_scope**:
  - Switching any non-FOW caller to `Format_RGBA8888` or to the no-flip vertex path — these may be follow-up work but are not in this chunk.
  - Adding caching to non-FOW `QGraphicsItem` instances (deferred).
  - Replacing `getImage()` composition (texture+FOW) with a GL-side composite — texture compositing remains CPU-side this chunk.

# Architecture Review

## Pre-Implementation Review — 2026-05-16T00:00:00Z

- **reviewer_model**: opus
- **verdict**: Revise
- **summary**: The plan misplaces GL state-mutating calls onto the GUI thread; `updateFowInternal()` and the proposed coalescer timer slot have no current OpenGL context, so the proposed `_fowGLObject->updateImage(...)` / `updateImageRegion(...)` invocations would silently no-op via the existing `currentContext()` guards and break the player-window FOW update. Chunk 4 also removes the shared `convertToFormat(RGBA8888)` from `loadTexture()` without addressing the other (non-FOW) callers that pass `Format_ARGB32_Premultiplied` images through the same code path.

- **triggers_evaluated**:
  - threading / GL context boundary: **concern** — chunks 1, 3, 4 route GL upload calls through GUI-thread functions (`updateFowInternal`, coalescer timer slot) where `QOpenGLContext::currentContext()` is null; chunk 4's stated mitigation ("let the player window pick up the update on its next paint") is incompatible with the actual code path proposed, which performs the GL upload at the call site rather than deferring to `playerGLPaint`.
  - layer_interface: addressed — DM path (`dmInitialize` + `FowGraphicsItem`) and player path (`playerGLInitialize` / `playerGLPaint`) are both maintained; each is independently functional after the threading defect is resolved.
  - serialization_shape: not-applicable — no XML format change; the plan correctly states this.
  - subsystem_boundary: addressed — battle subsystem only; `FowGraphicsItem` is a scoped helper.
  - new_subsystem_or_flag: not-applicable — no new `dmconstants.h` flag, no new top-level subsystem.

- **findings**:
  - Critical: chunk 1 (`gl-upload-reuse`) — `LayerFow::updateFowInternal()` is invoked from `paintFoWPoint` / `paintFoWPoints` / `paintFoWRect` / `paintFoWPolygon` / `fillFoW` / `applyPaintTo` / `editSettings`, all of which run on the GUI thread with no current GL context. Replacing `delete _fowGLObject` with `_fowGLObject->updateImage(getImage())` does not preserve behaviour: `PublishGLBattleBackground::loadTexture()` bails on `!QOpenGLContext::currentContext()` and returns without uploading. The current code's `delete` works because it sets `_fowGLObject = nullptr` and the renderer later re-calls `playerGLInitialize()` from a `*GL*` function; the proposed replacement removes that recreation trigger and therefore stops the player-side FOW from updating during strokes. cpp-qt.instructions.md "GL context rules" require GL calls to live inside functions whose name contains `GL`; `updateFowInternal()` does not.
  - Critical: chunk 4 (`subregion-upload`) — `updateImageRegion(...)` is proposed to be called from `LayerFow`'s dispatch path (which the chunk-3 design places inside the coalescer's timer slot, on the GUI thread). The chunk's own `constraints_in_scope` notes "`updateImageRegion` is only callable while a GL context is current", and the chunk-3 integration_tasks resolve the missing context with "let the player window pick up the update on its next paint" — but the proposed code path performs the GL call at the GUI-thread call site, not from `playerGLPaint`. Same GL context violation as chunk 1.
  - High: chunk 4 (`subregion-upload`) — `PublishGLBattleBackground::loadTexture()` is shared by all callers (FOW, battle background, and any token/effect background instances). Removing the `convertToFormat(QImage::Format_RGBA8888)` and `flipped(Qt::Vertical)` calls plus inverting V coordinates in the shared `createImageObjects` vertex layout changes the contract for **every** caller: they must now supply RGBA8888 source images, and any caller previously relying on the CPU flip will be visually correct only if it also stopped flipping. The plan defers verification to "document a follow-up in the chunk's handoff note" — that is insufficient; other callers may render byte-swizzled colours or vertically mirrored. Verification of every `PublishGLBattleBackground` construction site (and the format of the image each passes) must occur before merge, not after.
  - High: chunk 3 (`fow-graphics-item`) — `dispatchFowUpdate(region)` is described as calling `_fowGLObject->updateImage(getImage())` inline. Same GL-context defect: the dispatch runs from the coalescer timer slot on the GUI thread. The signal `fowRegionChanged` (DM-side `update(rect)`) is fine, but the GL-side branch must be deferred to a `*GL*` function.
  - High: chunk 1 (`gl-upload-reuse`) integration_tasks asserts "Calls into `_fowGLObject` from `updateFowInternal()` are reached only via the player paint thread when the GL object exists" — this is factually wrong (inherited from the investigation's parenthetical "delete `_fowGLObject` runs on the GL thread"). The plan needs a corrected risk statement and a corrected dispatch design.
  - Medium: chunk 3 (`fow-graphics-item`) — `dispatchFowUpdate` calls `getImage()`, which still composes the full `_imageFow + _imageFowTexture` image regardless of rect size. The dirty-rect optimisation is therefore only realised on the GL side (after chunk 4) and the DM-side `drawImage(exposedRect, *_image, exposedRect)`; the composition itself remains O(map_pixels). Not blocking, but worth a sentence in the plan so Execution does not believe rect-scoping eliminates the per-stroke composite cost.
  - Medium: chunk 4 (`subregion-upload`) — the plan correctly names `BYTES_PER_PIXEL_RGBA = 4` and the mipmap-filter predicate but does not require naming the V-coordinate value swap (1.0 ↔ 0.0). Per cpp-qt.instructions.md, the new V values should be named or at minimum carry an inline comment documenting the GL bottom-left origin convention; the plan only mandates the latter. Acceptable but should be explicit.
  - Low: chunk 2 (`coalesce-and-defer-dirty`) — `dirty()` semantics handled correctly (timer slot is silent on `dirty()`; mouse-up flush paths preserve emission). Signal-semantics rule respected. No issue.

- **required_plan_changes**:
  - gl-upload-reuse: Redesign the chunk-1 upload trigger. `updateFowInternal()` must not call `_fowGLObject->updateImage(...)` directly. Instead introduce a "pending GL upload" flag (and the necessary state — likely just a `bool _fowGLImageDirty` member or, anticipating chunk 4, a pending `QImage` / pending `QRect`) on `LayerFow`. The actual `updateImage(...)` call moves into `playerGLPaint()` (or a private `*GL*` helper invoked from it) where a GL context is current. Update the chunk's `acceptance_criteria` to reflect the deferred-upload contract, and remove the incorrect "reached only via the player paint thread" statement from `integration_tasks`.
  - coalesce-and-defer-dirty: Carry the deferred-upload contract through to the timer slot — the slot accumulates state and schedules a player-window repaint (`emit changed()` or whatever wakes the player renderer), but performs no GL calls. Document this explicitly so Execution does not regress to inline GL upload.
  - fow-graphics-item: Update `dispatchFowUpdate(region)` to (a) update `_pendingDirtyRect` / mark `_fowGLImageDirty`, (b) emit `fowRegionChanged(region)` for the DM-side `update(rect)`, and (c) wake the player renderer. The GL-side `updateImage(...)` call moves into `playerGLPaint()` and consumes the pending state. Update the `constraints_in_scope` text that currently says "the timer slot must check `_fowGLObject != nullptr` and let the player window pick up the update on its next paint" — replace with an explicit contract: the timer slot performs no GL calls; the next `playerGLPaint()` consumes pending state.
  - subregion-upload: (a) Move the `updateImageRegion(...)` invocation into `playerGLPaint()` so it always runs with a current GL context; rewrite the chunk's `constraints_in_scope` GL-context bullet to reflect this. (b) Add a pre-merge integration task enumerating every caller of `PublishGLBattleBackground` (battle background, tokens, any other) and verifying the source-image format and orientation each one passes; either keep a guarded `convertToFormat` fast path inside `loadTexture()` (no-op when already RGBA8888) so non-FOW callers remain correct, or update each caller to supply RGBA8888 in the same chunk. The "document as follow-up in commit message" approach is insufficient for a shared function and must be removed from `integration_tasks`.
  - global: Add an explicit note in `# Architectural Risk Assessment` that the investigation's parenthetical "delete `_fowGLObject` runs on the GL thread" is incorrect — `updateFowInternal()` and all FOW paint methods execute on the GUI thread with no current GL context — and that this is the central constraint driving the deferred-upload pattern across all four chunks.

# Cycle Log

## Chunk: gl-upload-reuse

### Cycle 1

- **dispatched_by**: coordinator
- **dispatched_at**: 2026-05-17T00:00:00Z
- **executor_files_touched**: (pending)
- **executor_build_status**: (pending)
- **executor_handoff_summary**: (pending)
- **review_verdict**: (pending)
- **review_findings**: (pending)
- **next_action**: (pending)

# Escalations

(none)

