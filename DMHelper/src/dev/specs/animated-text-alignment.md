# Spec: Fix font size and alignment in animated text renderer

## Summary

The player-facing publish window for animated (scrolling) text encounters
renders text at the wrong apparent font size and with incorrect alignment
relative to the full window. The defect was investigated and the root cause
identified. This spec describes the required user-visible behaviour and the
decisions already taken about alignment semantics and rasterisation strategy.

## User-visible behaviour

1. **Font size matches the editor.** When a DM sets font size to, say, 24pt in
   the text encounter editor, the scrolling text on the player screen should
   appear at 24pt relative to the publish window size — matching the user's
   expectation at every window resize.

2. **Alignment is relative to the full publish window.** Left-, center-, and
   right-aligned text aligns to the left/center/right edge of the publish
   window, not to the narrowed text band defined by `textWidth%`. The
   `textWidth%` slider controls the *width of the text band*, but alignment
   within that band is still anchored to the full window.

3. **Text works correctly whether or not a background layer is present.**
   Adding a background image/video layer to a text encounter must not change
   the apparent font size or alignment.

4. **Rotation (0/90/180/270) works correctly** in all combinations of the
   above.

5. **Live edits update the player window immediately.** Changing font size,
   alignment, or other formatter options in the DM editor while the encounter
   is being published immediately updates the player screen, preserving the
   current scroll position without a visible jump.

## Subsystems

- **UI shell** (`EncounterTextEdit`, `TextEditFormatterFrame`) — rasterisation
  pipeline and live-edit wiring.
- **Publish / GL** (`PublishGLTextRenderer`) — texture positioning, projection,
  and model-matrix scaling when layers are present.

## Root cause (summary)

- The DM-side `EncounterTextEdit` rasterises the text `QImage` in
  **target-widget pixels**, sized as `windowWidth × textWidth%`.
- The player-side `PublishGLTextRenderer::recreateContent()` positions the
  texture in **scene units**, offset by
  `-(sceneWidth × textWidth%) / 2`. When a background layer is present,
  scene units ≠ target pixels, so the texture is displayed at the wrong size
  and off-centre.
- `getRotatedWidth()`/`getRotatedHeight()` in the renderer ignore rotation
  when a layer scene is present, breaking 90°/270° completely in that case.
- Alignment is baked into the `QTextDocument` at the narrower `textWidth%`
  band, so right-aligned text never reaches the window edge.

## Design decisions (locked)

1. **Alignment semantic (Option A):** Always rasterise the `QImage` at the
   full rotated window width. Apply `textWidth%` as left/right root-frame
   margins on the `QTextFrameFormat` so the text band is centred and
   alignments are relative to the window.
2. **Rasterisation at publish-window pixels:** The `QImage` is always sized
   to the publish-window pixel dimensions (rotated). When a background layer
   is present and its scene size differs from the window, the renderer scales
   the text texture's model matrix by `sceneWidth / windowWidth` so it spans
   the full scene area while keeping the user's chosen font size.
3. **Live updates preserve scroll position:** `setTextImage` must not reset
   `_textPos`. When the new image's height differs from the old one, scale
   `_textPos` proportionally so the same visual line remains at the viewport
   centre.

## Done conditions

- Animated text encounter on a publish window with no layers: all four
  alignments (left, center, right, justify) and all four rotations display
  correctly; font size is consistent across window resizes.
- Same encounter with a background image layer: font size and alignment
  unchanged.
- `textWidth% = 50`, alignment = right: text band right-edge aligns to the
  right half of the window (i.e. to the window edge minus the 25% right
  margin imposed by the text band).
- Changing font size or alignment in the DM editor while publishing: player
  window updates without scroll-position jump.
- Build succeeds with no new warnings.

## Open questions

None — all design decisions have been taken by the human.

## Files in scope

- `DMHelper/src/publishgltextrenderer.cpp` / `.h`
- `DMHelper/src/encountertextedit.cpp` / `.h`

Read-only references (do not modify):
- `DMHelper/src/encountertext.h`
- `DMHelper/src/publishglscene.cpp`
- `DMHelper/src/publishglimage.cpp`
- Other `PublishGL*Renderer` files (rotation pattern reference)
