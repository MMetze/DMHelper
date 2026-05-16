# Deep Dive: Fog-of-War interactive edit pipeline performance

date: 2026-05-16
investigator_model: Claude Opus 4.7
focus: End-to-end latency and frame-rate of FOW erase/draw while the left mouse button is held in edit mode, with the player (GL) window publishing.
verdict: Warn

## Summary

The FOW edit hot path is structured as if each stroke point were a discrete,
authoritative scene change: every mouse-move synchronously rebuilds a full-size
QPixmap, replaces the DM scene item's pixmap, deletes the GL texture object,
and on the next player frame allocates a new VAO/VBO/EBO/texture, deep-copies
the entire FOW QImage twice (format conversion + vertical flip), and rebuilds
the full mipmap pyramid. There is no dirty-rect tracking, no coalescing, no
`glTexSubImage2D`, no item cache, and `dirty()` (which marks the campaign
modified and cascades up to `EncounterBattle`) fires per mouse-move sample.

None of this is a crash or correctness defect, so the overall verdict is
**Warn**. But the cumulative cost scales as O(map_pixels) **per mouse sample**,
which is why the regression worsens with map size exactly as you described.
The recommended fix is structural: a dedicated `QGraphicsItem` subclass that
owns the QImage and supports partial `update(QRectF)`, paired with persistent
GL texture + `glTexSubImage2D`, plus a 16 ms coalescer that batches both the
DM repaint and the player upload.

## Findings

### F1. `updateFowInternal()` runs on every mouse-move sample and rebuilds the full pixmap — Warn

file: [layerfow.cpp](layerfow.cpp#L756-L765)
category: performance

```cpp
void LayerFow::updateFowInternal()
{
    QImage newImage = getImage();

    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));

    if(_fowGLObject)
    {
        delete _fowGLObject;
        _fowGLObject = nullptr;
    }
}
```

`paintFoWPoint()` calls this unconditionally when `_batchProcessing` is false
([layerfow.cpp](layerfow.cpp#L316-L354)), and `UndoFowPath::addPoint()` calls
`paintFoWPoint()` on every mouse-move sample
([undofowpath.cpp](undofowpath.cpp#L67-L72)). The image is
`Format_ARGB32_Premultiplied` ([layerfow.cpp](layerfow.cpp#L724)), so
`QPixmap::fromImage` itself is cheap on raster, but `getImage()` composes
`_imageFow` against `_imageFowTexture` (full-image work), and
`QGraphicsPixmapItem::setPixmap` invalidates the item's entire bounding rect,
forcing the scene to repaint every overlapping item beneath FOW (background,
grid, tokens, effects, lighting). For a 4k×4k map covering the viewport,
that's a viewport-wide repaint per mouse sample.

**Recommended action:** Replace `setPixmap()` with a custom
`QGraphicsItem` subclass that owns the QImage by reference and calls
`update(dirtyRectF)` for only the brush footprint; see Recommendations.

---

### F2. GL texture object destroyed and rebuilt from scratch per stroke sample — Warn

file: [layerfow.cpp](layerfow.cpp#L762-L765), [publishglbattlebackground.cpp](publishglbattlebackground.cpp#L68-L88)
category: performance

```cpp
// layerfow.cpp
if(_fowGLObject)
{
    delete _fowGLObject;
    _fowGLObject = nullptr;
}
```

```cpp
// publishglbattlebackground.cpp
void PublishGLBattleBackground::setImage(const QImage& image)
{
    cleanup();              // glDeleteVertexArrays, glDeleteBuffers, glDeleteTextures
    createImageObjects(image);  // glGenVertexArrays, glGenBuffers, glGenTextures, loadTexture
    updateModelMatrix();
}
```

The next `playerGLPaint()` sees `_fowGLObject == nullptr` and reconstructs
everything: VAO, VBO, EBO, texture, vertex upload, and full `glTexImage2D`.
Note that `PublishGLBattleBackground::updateImage()` already has a
same-size fast path that calls `loadTexture()` without destroying the GL
objects — but LayerFow bypasses it by always going through full destroy
instead of calling `updateImage()`. This is gratuitous: FOW image size never
changes during a stroke.

**Recommended action:** Keep `_fowGLObject` alive across edits; route updates
through `updateImage()` (or a new `updateImageRegion()`); destroy only on
resize or layer teardown.

---

### F3. `loadTexture()` deep-copies the entire FOW image twice per upload — Warn

file: [publishglbattlebackground.cpp](publishglbattlebackground.cpp#L189-L213)
category: performance

```cpp
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).flipped(Qt::Vertical);
#else
QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
#endif
f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(),
                0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
```

`_imageFow` is `Format_ARGB32_Premultiplied`, so `convertToFormat(RGBA8888)`
is a full byte-swizzle deep copy of the entire image (~64 MB for 4k×4k).
`flipped()` is a second full deep copy. Both happen once per stroke sample
right now because of F2. Even with F2 fixed, these copies remain the dominant
single-frame cost.

**Recommended action:** Either (a) flip the texture's V coordinate in the
shader / vertex layout instead of CPU-flipping the image, and store
`_imageFow` directly in `Format_RGBA8888` (still works with `QPainter`);
or (b) use `glPixelStorei(GL_UNPACK_ROW_LENGTH, ...)` + `glTexSubImage2D`
to upload only the dirty stripe without copying. Option (a) eliminates both
copies permanently.

---

### F4. `glGenerateMipmap` rebuilds the full mipmap pyramid every upload — Warn

file: [publishglbattlebackground.cpp](publishglbattlebackground.cpp#L213)
category: performance

```cpp
f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ..., glBackgroundImage.bits());
f->glGenerateMipmap(GL_TEXTURE_2D);
```

Mipmap generation on a 4k×4k RGBA texture is a driver-side full reduction
through ~12 levels. The FOW filter is `GL_NEAREST`
([layerfow.cpp](layerfow.cpp#L670)), which never samples beyond level 0,
so the mipmap is built and immediately unused.

**Recommended action:** Don't call `glGenerateMipmap` when `_textureParam`
is `GL_NEAREST` (or any non-mipmap filter). If mipmaps are needed for
non-FOW callers, gate this on filter type, and regenerate at most on stroke
end (mouse-up), not per sample.

---

### F5. Every paint method touches the full-size `_imageFow`; no dirty-rect propagation — Warn

file: [layerfow.cpp](layerfow.cpp#L316-L544)
category: performance

`paintFoWPoint`, `paintFoWPoints`, `paintFoWRect`, `paintFoWPolygon`,
`fillFoW` all construct a `QPainter` on `_imageFow` and immediately call
`updateFowInternal()` — but they never return or store the affected rect.
Downstream consumers (DM pixmap item, GL texture) therefore have no way
to know which sub-region actually changed and must repaint/upload the whole
image.

**Recommended action:** Have each paint method compute a tight `QRect` of
its own footprint (radius for points, bounding rect for shape/polygon) and
expose it via a new signal `fowRegionChanged(const QRect&)` (separate from
`dirty()`). The DM item and GL uploader consume the rect; `dirty()` keeps
its current "campaign modified" semantics. The brush footprint for a
typical stroke is ~50×50 px — three orders of magnitude smaller than a 4k
map.

---

### F6. `dirty()` fires per mouse-move sample and cascades to `EncounterBattle` — Warn

file: [layerfow.cpp](layerfow.cpp#L354,L445,L505,L531,L544), [layerscene.cpp](layerscene.cpp#L822-L829), [battledialogmodel.cpp](battledialogmodel.cpp#L39-L42)
category: performance

```cpp
// layerscene.cpp
connect(layer, &Layer::dirty, this, &LayerScene::handleLayerDirty);

// battledialogmodel.cpp
connect(this, &BattleDialogModel::dirty, _encounter, &EncounterBattle::dirty);
connect(&_layerScene, &LayerScene::dirty, this, &BattleDialogModel::dirty);
```

Each stroke sample emits `dirty()`, which walks `LayerScene` → `BattleDialogModel`
→ `EncounterBattle`. Per CLAUDE.md `dirty()` is "unsaved data changed" — it's
the right signal *eventually*, but emitting per sample (~100 Hz on a fast
mouse) does redundant work in every downstream slot, and your own answer (#4)
confirmed mouse-up is acceptable.

**Recommended action:** In `UndoFowPath::addPoint()` / per-sample paint calls,
suppress `dirty()` and emit it only from `BattleFrameMapDrawer::endPath()` and
`MapFrame`'s equivalent. Keep `dirty()` on `fillFoW`, `paintFoWRect`,
`paintFoWPolygon` (those are atomic, not streaming).

---

### F7. DM `QGraphicsView` uses default settings; no cache mode on items — Warn

file: [battledialoggraphicsview.cpp](battledialoggraphicsview.cpp#L1-L18), [layerfow.cpp](layerfow.cpp#L630-L641)
category: performance

The view sets no `setViewportUpdateMode`, `setOptimizationFlags`, or
`setCacheMode`. The FOW pixmap item also has no cache mode set, so every
`setPixmap` invalidation results in a re-rasterization of the pixmap into
the viewport on the next paint event, on top of repainting all overlapping
items. Combined with F1, this is the source of the "DM view stutters during
stroke" symptom.

**Recommended action:** With the custom `QGraphicsItem` from F1 in place,
set `QGraphicsItem::DeviceCoordinateCache` on items that don't change every
frame (background, grid). The FOW item itself should *not* cache, because
its content actively mutates — instead it should call `update(dirtyRect)`
with a tight rect so the scene only repaints the brush footprint.
`QGraphicsView::setViewportUpdateMode(QGraphicsView::SmartViewportUpdate)`
is the default in Qt 6 and is correct; verify it's not being changed
elsewhere.

---

### F8. No coalescing between mouse-event rate and frame rate — Warn

file: [battleframemapdrawer.cpp](battleframemapdrawer.cpp#L95-L114), [layerfow.cpp](layerfow.cpp#L756-L765)
category: performance

`handleMouseMoved` synchronously runs the full pipeline. Qt 6 typically
delivers `mouseMoveEvent` per OS sample (up to 1000 Hz on a gaming mouse,
typically 125 Hz). The DM view paints at the compositor rate (~60 Hz); the
player GL window paints when invalidated. Doing the full pixmap rebuild and
texture upload work 2–10× per displayed frame is pure waste. Confirmed there
is no QTimer-based coalescer anywhere in the FOW path (`paintGL` of the
player window is driven only by `update()` calls, no `_targetFPS` timer in
`publishgl*` other than `publishgltextrenderer`).

**Recommended action:** Introduce a single-shot coalescing `QTimer` on
`LayerFow` (e.g. 16 ms / configurable). Per-sample paint accumulates into
`_imageFow` and a running dirty `QRect`, then schedules the timer if not
already pending. The timer fires `fowRegionChanged(rect)` exactly once per
~16 ms window. Mouse-up flushes immediately. This decouples sample rate from
both the DM repaint and the GL texture upload.

---

## Verdict Rationale

No findings are crash-path, data-loss, race-condition, or undefined-behaviour
issues. Every finding is a steady-state performance defect. The closest call
is F2 — a stroke-rate destroy/create cycle of GL objects could in principle
exhaust resources or expose a context-affinity bug, but no evidence of either
was found and `delete _fowGLObject` runs on the GL thread (per CLAUDE.md GL
context rules in `playerGL*` functions). Overall verdict: **Warn**.

The defects compound multiplicatively: F1 + F2 + F3 + F4 + F5 means each
mouse sample on a 4k map does roughly:
- 1 full-image composite (`getImage()`)
- 1 full-image QImage→QPixmap (cheap, premultiplied)
- 1 GL teardown + alloc cycle
- 2 full-image deep copies (format + flip)
- 1 full `glTexImage2D` upload (~64 MB)
- 1 full mipmap pyramid generation
- 1 full-viewport DM scene repaint
- 1 cascaded `dirty()` chain into the campaign model

This is consistent with your report that "the larger the map, the more
pronounced", and that publishing makes it worse (F2–F4 only run when the
player window is open).

## Next Steps

Suggested implementation order, smallest-blast-radius first. Each step is
independently shippable and measurable:

1. **F4 first (smallest patch, instant win on player side).** Gate
   `glGenerateMipmap` on a mipmap-needing filter. Removes one of the largest
   per-upload costs immediately, no architectural change.

2. **F2.** Stop deleting `_fowGLObject`. Route updates through
   `PublishGLBattleBackground::updateImage()` which already has a same-size
   fast path. Costs a small refactor of `updateFowInternal()`.

3. **F6.** Stop emitting `dirty()` from per-sample paints; emit from
   `endPath()` and shape/fill operations only.

4. **F8.** Add a 16 ms coalescing single-shot `QTimer` to `LayerFow` that
   batches updates. Mouse-up flushes synchronously. Pairs naturally with F2.

5. **F5 + F1 + F7 (architectural).** Introduce `FowGraphicsItem :
   QGraphicsItem` that owns the `QImage` by reference, implements `paint()`
   with `painter->drawImage(option->exposedRect, _image, option->exposedRect)`,
   and accepts `updateRegion(QRectF)`. Add `fowRegionChanged(QRect)` signal
   from `LayerFow` with the brush footprint. The DM item consumes it via
   `update(rect)`; the coalescer accumulates rects for one-shot dispatch.
   This is the structural change that unblocks proportional cost (cost
   scales with brush size, not map size).

6. **F3.** Switch `_imageFow` storage to `Format_RGBA8888` and move the
   vertical flip into the vertex/shader stage. Then convert F2's
   `updateImage` path to `glTexSubImage2D(_textureID, dirty.x, dirty.y, ...)`
   using `glPixelStorei(GL_UNPACK_ROW_LENGTH, _imageFow.bytesPerLine()/4)`
   so only the dirty stripe is uploaded.

Together these reduce per-sample cost from O(map_pixels) to O(brush_pixels)
on both DM and player paths, and reduce per-displayed-frame work to one
update regardless of mouse sample rate.

Validation:
- Profile a 30-second stroke on a 4k map with and without the player window
  open, on Windows debug + release. Look at the DM view's
  `QGraphicsScene::changed` rate and GL upload count.
- Confirm undo/redo still produces identical FOW imagery (F5's rect-tracking
  must not break `applyPaintTo()` replay; the replay path uses
  `_batchProcessing` and already calls `updateFowInternal()` once at the
  end, which should map cleanly to a single full-image dispatch).
- macOS sanity check after F3 — `Format_RGBA8888` storage interacts
  differently with CoreGraphics-backed QPainter.

No Coordinator escalation needed (Warn verdict). The work above naturally
forms a single-chunk Design Agent plan or a multi-step Coordinator
dispatch — your call which path to take.
