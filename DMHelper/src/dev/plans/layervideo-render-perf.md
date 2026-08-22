---
feature_slug: layervideo-render-perf
spec_path: (inline — see Findings section)
created: 2026-08-20
designer_model: claude (interactive session)
arch_review_required: true
arch_review_model: opus
arch_review_reason: Touches the OpenGL upload pipeline (texture storage format, per-frame glTexSubImage2D path, GL_UNPACK_ROW_LENGTH state) and the `LayerVideo` Layer subclass, plus the libvlc decode-format contract in `VideoPlayer::formatCallback` consumed from a VLC worker thread.
pre_impl_arch_review_requested: true
status: draft
---

# Summary

LayerVideo playback in the player window is choppy at full-screen even in
release builds. The render path is CPU-bound on the GUI thread: for every
decoded frame it performs three full-frame deep copies plus a full texture
reallocation. This plan (a) adds permanent, cheap instrumentation so the
problem and every fix are objectively measurable, then (b) removes all
three copies and the reallocation in three small chunks, reusing the
fast-path infrastructure already built for the FOW pipeline
(`sourceIsRgba8888` / `sourceNeedsVerticalFlip` flags and
`updateImageRegion()` on `PublishGLBattleBackground`).

Subsystems touched: video playback (`VideoPlayer`), the video layer
(`LayerVideo`), and the player-window GL upload path
(`PublishGLBattleBackground`). No serialization change. No `.ui` / `.qrc`
changes. `LAYERVIDEO_USE_OPENGL` remains disabled (see Deferred
Opportunities).

# Findings

Per-frame cost of the current path, measured in code (1080p BGRA frame
≈ 8.3 MB; all on the GUI/render thread inside `playerGLPaint`):

| # | Site | Cost |
|---|------|------|
| F1 | `layervideo.cpp` `playerImage->copy()` | 8.3 MB deep copy |
| F2 | `publishglbattlebackground.cpp` `loadTexture()` `convertToFormat(Format_RGBA8888)` | 8.3 MB copy + CPU BGRA→RGBA swizzle |
| F3 | `loadTexture()` `flipped(Qt::Vertical)` | 8.3 MB copy |
| F4 | `updateImage()` same-size path calls `loadTexture()` → full `glTexImage2D` **reallocation** every frame instead of `glTexSubImage2D` | driver alloc + 8.3 MB upload |

Total ≈ 33 MB touched per frame ≈ 1 GB/s at 30 fps on one thread; 4K
content is 4×. The decode side (`VideoPlayer`) is already efficient:
triple-buffered, pitch-aligned, VLC performs the chroma conversion on its
own worker thread. The waste is entirely between `getLockedImage()` and
the GL upload.

Supporting facts:

- `VideoPlayer::formatCallback` currently requests `"BGRA"` chroma and
  wraps buffers as `QImage::Format_ARGB32`. VLC's converter can produce
  `"RGBA"` (= `Format_RGBA8888`) at identical cost on the VLC thread,
  making F2 free.
- `PublishGLBattleBackground` already supports
  `sourceIsRgba8888` / `sourceNeedsVerticalFlip` constructor flags
  (V-coordinate inversion instead of CPU flip) and a pitch-aware
  `updateImageRegion()` (`GL_UNPACK_ROW_LENGTH` + `glTexSubImage2D`),
  built for the FOW effort. The video path simply doesn't use them yet.
- The display buffer returned by `getLockedImage()` is consumer-exclusive
  (triple buffering: write/ready/display), so the GL upload can read it
  directly without a defensive copy.
- Consumers affected by a decode-format switch: `LayerVideo` (this plan),
  `PublishGLBattleEffectVideo` (calls `convertToFormat(Format_RGBA8888)`
  explicitly — becomes a near-no-op, stays correct),
  `VideoPlayerScreenshot` (format-agnostic QImage save).

Expected net effect of chunks 2–4: per-frame work drops from ~33 MB across
four passes to a single ~8.3 MB `glTexSubImage2D` upload — roughly 4×
less render-thread work, with no new abstractions.

# Architectural Risk Assessment

- **Trigger 1 (threading / GL context boundary): Hit.** Chunk 2 changes
  the format contract filled in by `formatCallback` on a VLC worker
  thread. Chunk 4 adds a `GL_UNPACK_ROW_LENGTH` path to `loadTexture()`
  and switches per-frame uploads to `updateImageRegion()`. All GL calls
  remain inside `playerGLPaint()`; no new GUI-thread GL call sites.
- **Trigger 2 (Layer subclass): Hit.** Chunks 1, 3, 4 modify `LayerVideo`.
- **Trigger 3 (serialization): Not hit.**
- **Trigger 4 (>1 of battle/audio/campaign/UI shell): Not hit.**
- **Trigger 5 (feature flags): Not hit.** `LAYERVIDEO_USE_OPENGL` stays
  disabled; this plan explicitly does not touch the `videoplayergl*` path.

# Chunks

## Chunk 1: Instrumentation — make choppiness measurable

- **id**: video-perf-stats
- **summary**: Add cheap counters and timers so decode rate, upload rate,
  dropped frames, per-stage cost, and frame-pacing jitter are reported
  objectively. All statistics gated behind a compile-time flag
  (`#define LAYERVIDEO_PERF_STATS`, on by default while this plan is in
  flight) so release measurement is possible; reporting is a periodic
  `qDebug` line, never per-frame.
- **dependencies**: []
- **branch**: agent/work/video-perf-stats
- **files_to_modify**:
  - videoplayer.h / videoplayer.cpp — expose counters: frames decoded
    (`displayCallback` count), frames dropped at decode (fail-forward
    events in `lockCallback` / contention drops in `unlockCallback`).
    Getters only; no behavioural change.
  - layervideo.cpp — in `playerGLPaint`: count uploads; `QElapsedTimer`
    around each stage (copy / convert+flip inside `updateImage` measured
    as one "upload" span initially); record ms since previous upload for
    jitter (min/avg/max). Emit one summary `qDebug` line every
    `STATS_REPORT_INTERVAL_MS` (named constant, e.g. 5000) and reset.
- **files_to_create**: []
- **integration_tasks**:
  - Dropped frames = decoded − uploaded over the report window; jitter
    max ≫ avg identifies pacing problems vs raw throughput problems.
  - Counters incremented from VLC threads must be `std::atomic`;
    aggregation/reporting happens only in `playerGLPaint` (GUI thread).
  - No `dirty()` emissions; stats are visual-diagnostic only.
- **acceptance_criteria**:
  - With the flag on, playing a video logs a periodic one-line report:
    decoded, uploaded, dropped, avg/max upload ms, min/avg/max frame
    interval ms.
  - With the flag off, the added code compiles to nothing (or to inert
    atomic increments — no timers, no logging).
  - No new GL calls; no behaviour change to playback.
  - Windows debug + release builds succeed.
- **constraints_in_scope**:
  - Named constants for the report interval per cpp-qt.instructions.md.
  - VLC callbacks must not touch Qt GUI objects (CLAUDE.md threading rule).
- **out_of_scope**: Any optimization; GPU timer queries; on-screen overlay.

## Chunk 2: Decode directly to RGBA (kills F2)

- **id**: rgba-chroma-decode
- **summary**: In `VideoPlayer::formatCallback`, request `"RGBA"` chroma
  instead of `"BGRA"` and construct `VideoPlayerImageBuffer` QImages as
  `Format_RGBA8888` instead of `Format_ARGB32`. VLC's converter performs
  the swizzle on its worker thread at unchanged cost; the GUI-thread
  `convertToFormat` in `loadTexture()` becomes skippable (consumed in
  chunk 3).
- **dependencies**: [video-perf-stats]
- **branch**: agent/work/rgba-chroma-decode
- **files_to_modify**:
  - videoplayer.cpp — `formatCallback`: `memcpy(chroma, "RGBA", ...)`;
    `VideoPlayerImageBuffer` ctor: `QImage::Format_RGBA8888`.
- **files_to_create**: []
- **integration_tasks**:
  - Verify at runtime that VLC 4 accepts the `"RGBA"` fourcc through the
    format callback (it is a standard VLC chroma; confirm via chunk-1
    stats that decoded-frame rate is unchanged).
  - Audit the two other frame consumers:
    `PublishGLBattleEffectVideo::prepareObjectsGL/paintGL` call
    `convertToFormat(Format_RGBA8888)` explicitly → still correct, now
    cheaper. `VideoPlayerScreenshot::handleScreenshot` saves via QImage →
    format-agnostic. No code change expected in either; confirm by
    reading, not assuming.
- **acceptance_criteria**:
  - Video plays with correct colors (no R/B swap) in DM preview,
    player-window publish, and video effect tokens; screenshots/cached
    thumbnails keep correct colors.
  - Chunk-1 stats show decoded-frame rate unchanged vs baseline.
  - No remaining `"BGRA"` literal in videoplayer.cpp.
- **constraints_in_scope**: VLC callback threading rules unchanged.
- **out_of_scope**: Any change to `PublishGLBattleBackground`;
  `mapvideothumbnailgrabber.cpp` (separate RV32 pipeline, unaffected).

## Chunk 3: Skip convert + flip in the video GL object (kills F3, consumes F2)

- **id**: video-gl-upload-flags
- **summary**: `LayerVideo::playerGLPaint` constructs its
  `PublishGLBattleBackground` with `sourceIsRgba8888 = true` and
  `sourceNeedsVerticalFlip = false`, reusing the FOW fast-path flags. The
  V-coordinate inversion in `createImageObjects()` replaces the CPU flip.
- **dependencies**: [rgba-chroma-decode]
- **branch**: agent/work/video-gl-upload-flags
- **files_to_modify**:
  - layervideo.cpp — both `new PublishGLBattleBackground(...)` call sites
    in `playerGLPaint` (creation branch) pass the two flags.
- **files_to_create**: []
- **integration_tasks**:
  - Verify orientation: VLC frames are top-down (row 0 = top), identical
    to the FOW image convention, so the existing inverted-V vertex path
    renders right-side-up. Confirm visually on publish.
  - The frame is still `copy()`d at this chunk (tight rows), so
    `loadTexture()`'s packed-row `glTexImage2D` remains correct; the
    pitch-aware path arrives in chunk 4.
- **acceptance_criteria**:
  - Published video is right-side-up and correctly colored.
  - Chunk-1 stats show avg upload ms reduced vs chunk-2 baseline
    (convert + flip copies eliminated).
- **constraints_in_scope**: GL calls stay inside `playerGLPaint`.
- **out_of_scope**: Removing the `copy()` (chunk 4); other
  `PublishGLBattleBackground` callers keep default flags.

## Chunk 4: Zero-copy upload via glTexSubImage2D (kills F1, F4)

- **id**: zero-copy-subimage-upload
- **summary**: Upload directly from the consumer-exclusive display buffer:
  drop `playerImage->copy()` in `LayerVideo::playerGLPaint`; for same-size
  frames call `updateImageRegion(*playerImage, fullRect)` (pitch-aware
  `glTexSubImage2D`) instead of `updateImage()` (full `glTexImage2D`
  realloc). Make `loadTexture()` stride-safe for the pitch-padded creation
  upload by setting `GL_UNPACK_ROW_LENGTH` from `image.bytesPerLine()`
  (and restoring 0), mirroring `updateImageRegion()`.
- **dependencies**: [video-gl-upload-flags]
- **branch**: agent/work/zero-copy-subimage-upload
- **files_to_modify**:
  - layervideo.cpp — `playerGLPaint`: creation branch passes
    `*playerImage` directly to the `PublishGLBattleBackground`
    constructor; update branch calls
    `_videoObject->updateImageRegion(*playerImage, QRect(QPoint(0, 0), playerImage->size()))`.
    Keep the `lockMutex()/unlockMutex()` bracket (no-ops today, cheap
    insurance if synchronization returns).
  - publishglbattlebackground.cpp — `loadTexture()`: set
    `GL_UNPACK_ROW_LENGTH = image.bytesPerLine() / BYTES_PER_PIXEL_RGBA`
    before `glTexImage2D`, restore to 0 after. For tightly packed callers
    row length equals width, so behaviour is unchanged for them.
- **files_to_create**: []
- **integration_tasks**:
  - Safety argument to document inline at the upload site: the display
    buffer is consumer-exclusive under the triple-buffer scheme
    (`_idxWrite`/`_idxReady`/`_idxDisplay`); the decoder never writes the
    display slot, so reading it during the GL upload without a copy is
    race-free. If the buffering scheme ever changes, this assumption must
    be revisited.
  - `updateImageRegion()` requires `Format_RGBA8888` — guaranteed by
    chunk 2. It reads `image.constScanLine()` honoring `bytesPerLine()`,
    so the pitch-padded VLC buffer uploads correctly.
  - Size-change handling (player restart / new media) must still route
    through `setImage()`/creation, not the sub-upload path: guard on
    `_videoObject->getSize() != playerImage->size()`.
- **acceptance_criteria**:
  - No `playerImage->copy()` remains in `layervideo.cpp`'s paint path.
  - Same-size frames upload via `updateImageRegion`; no per-frame
    `glTexImage2D` in the steady state (verify with chunk-1 stats:
    avg upload ms drops again; also verify visually: no flicker on
    looping restart or publish-window resize).
  - `GL_UNPACK_ROW_LENGTH` is restored to 0 after every upload that sets it.
  - Full ARM64 + x64 release builds succeed; publish playback smooth-ness
    validated with before/after stats on the same test video.
- **constraints_in_scope**:
  - All GL calls inside `playerGLPaint` (GL context rule).
  - No magic numbers — reuse `BYTES_PER_PIXEL_RGBA`.
- **out_of_scope**: PBO async uploads (revisit only if measurements still
  show upload cost dominating); decode-size capping (below); GL engine
  path (below).

# Deferred Opportunities (documented, not planned)

## Option 5: VLC-side decode scaling (target size)

`VideoPlayer::formatCallback` already supports scaling the decode target
(`_targetSize`, `Qt::KeepAspectRatio`), but `LayerVideo` constructs the
player with `QSize()` — so a 4K source is decoded, converted, and uploaded
at 4K even on a 1080p output. Passing the real output size would cut
decode-convert cost and upload bandwidth by ~4× for 4K-on-1080p.

Why deferred:
- `VideoPlayer::targetResized()` restarts the player — a window resize
  would cause a visible playback hiccup unless restart-free rescaling is
  designed.
- The publish window size is not known at layer/player creation time and
  can change (fullscreen toggle, monitor move); `LayerVideo` only learns
  it in `playerGLResize`.
- Multiple consumers (DM preview vs player window) would need per-consumer
  size policy.

Estimated payoff: large for oversized sources only; zero for sources at or
below output resolution. Measure source-vs-output resolutions with chunk-1
stats before investing.

## GL engine path (`videoplayergl*` + `LAYERVIDEO_USE_OPENGL`)

The endgame: VLC 4's `libvlc_video_set_output_callbacks(...,
libvlc_video_engine_opengl, ...)` renders frames into FBO textures on
VLC's own thread — zero CPU copies, zero GUI-thread upload, and GPU-side
scaling for free. The wrapper classes (`VideoPlayerGLPlayer`,
`VideoPlayerGLVideo`, `VideoPlayerGLScreenshot`) exist and their event API
was just migrated to VLC 4 (2026-08-20).

Known gaps blocking enablement (verified in code):
- `LayerVideo`'s `#ifdef LAYERVIDEO_USE_OPENGL` branches contain literal
  `TODO:` placeholder text that does not compile (`playerGLPaint`,
  `getLayerScreenshot` regions).
- Positioning/scene-rect mapping for the GL video quad is unimplemented
  (`TODO: update this to position correctly`, identity model matrix).
- Layer visibility handling in the GL branch is unimplemented.
- The classes have not tracked several seasons of `LayerVideo`/renderer
  changes; audio-track handling and restart logic contain stale TODOs.
- Depends on `Qt::AA_DontCheckOpenGLContextThreadAffinity` +
  `makeCurrent()` from the VLC thread — a known-delicate area (crash
  history in this subsystem).

Estimated payoff: removes the remaining ~8.3 MB/frame upload and all
render-thread frame work — the theoretical optimum. Recommendation:
re-evaluate only after chunks 1–4 land, using chunk-1 stats; if steady-
state upload cost is then negligible, the GL path's risk is not worth it,
and the flag stays off. If 4K/high-fps content still struggles, revive the
GL path as its own spec + plan with a full regression review of the three
`videoplayergl*` classes.

# Cycle Log

(empty — no chunks executed yet)
