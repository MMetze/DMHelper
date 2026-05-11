---
feature_slug: spellbook-effect-panel
spec_path: DMHelper/src/dev/specs/spellbook-effect-panel.md
created: 2026-05-10
designer_model: opus
arch_review_required: false
arch_review_model: sonnet
arch_review_reason: n/a
pre_impl_arch_review_requested: false
supersedes: null
status: complete
---

# Summary

`SpellbookTemplateDialog` is the new template-driven replacement for the legacy `SpellbookDialog`. Its `.ui` already contains the full effect panel (effect type combo, size fields, shape group with color/opacity, token line-edit + browse + rotate, conditions group with edit button and icon strip, and a shape preview label), but no code currently reads from or writes to those widgets. This feature ports the complete effect-panel behaviour from `spellbookdialog_OLD.cpp` into `SpellbookTemplateDialog`, adapted for the `Spellv2` API.

The port is a near-mechanical translation with three deliberate API differences from the OLD reference: (1) condition storage moves from the legacy `int` bitmask (`getEffectConditions` / `setEffectConditions`) to the `Spellv2` `QStringList` API (`getEffectConditionList` / `setEffectConditionList`); (2) `ConditionsEditDialog` is driven via `setConditionList` / `getConditionList`; (3) condition icons resolve via `Conditions::activeConditions()->getConditionIconPath(id)` rather than the old `Combatant::getConditionIcon()`. All other effect logic (size symmetry rules for Radius/Cone/Cube, token rotation, "2-Minute Tabletop" credit visibility, shape preview rendering, enable/disable based on effect type) ports directly.

Subsystems touched: UI shell, Spellbook. The TemplateFrame / `SpellbookFactory` data path is not modified — the effect controls are not declarative template fields and so live entirely in dialog code, parallel to the factory-driven name/level/school/etc. fields.

# Architectural Risk Assessment

- **Trigger 1 (threading / GL boundaries)**: Not hit. All work is in a `QDialog` on the GUI thread; no VLC, worker thread, or GL involvement.
- **Trigger 2 (Layer subclass)**: Not hit. `SpellbookTemplateDialog` is not a `Layer`.
- **Trigger 3 (serialization shape change)**: Not hit. `Spellv2` already exposes the full effect-field API; this feature only consumes existing getters/setters.
- **Trigger 4 (>1 of battle / audio / campaign / UI shell)**: Not hit. Touches UI shell (Spellbook dialog) only; no battle, audio, or campaign code paths are modified.
- **Trigger 5 (new top-level subsystem or new `dmconstants.h` flag)**: Not hit.

`arch_review_required = false`, `arch_review_model = sonnet`, `pre_impl_arch_review_requested = false`.

# Chunks

## Chunk 1: Wire effect panel to Spellv2

- **id**: effect-panel-wiring
- **summary**: Port the full effect-panel behaviour from `spellbookdialog_OLD.cpp` into `SpellbookTemplateDialog`, adapted to the `Spellv2` API; populate controls from the spell on `setSpell`, persist every control change back to the spell, render the shape/token preview, and display condition icons.
- **dependencies**: []
- **branch**: agent/work/effect-panel-wiring
- **files_to_modify**:
  - DMHelper/src/spellbooktemplatedialog.h — add `_tokenRotation` and `_conditionLayout` members; declare new slots and private helpers (`storeEffectData`, `loadEffectUI`, `updateEffectImage`, `updateConditionLayout`, `clearConditionLayout`, `addConditionIcon`, plus `handleEffectTypeChanged`, `handleEffectWidthChanged`, `handleEffectHeightChanged`, `handleTokenRotateCW`, `handleTokenRotateCCW`, `selectEffectToken`, `editEffectConditions`); add necessary forward declarations (`QHBoxLayout`).
  - DMHelper/src/spellbooktemplatedialog.cpp — initialise new members; in the constructor add the effect-panel signal/slot wiring (effect-type combo, width/height edits, color button, opacity slider, token line-edit, token browse, token rotate CW/CCW, shape group toggle, edit-conditions button); set validators and disabled-state stylesheets on the size and token edits; implement all new helpers and slots; have every effect-control change call `storeEffectData()` (and trigger `updateEffectImage()` / `updateConditionLayout()` as appropriate); have `setSpell()` call a new private `loadEffectUI()` to populate effect controls without writing back; include `<QFileDialog>`, `<QPainter>`, `<QHBoxLayout>`, `<QLabel>`, `<QIntValidator>`, `<QTransform>`, `"battledialogmodeleffect.h"`, `"conditionseditdialog.h"`, `"conditions.h"`, `"spellbook.h"` as needed.
- **files_to_create**: []
- **integration_tasks**:
  - In the constructor, after the existing nav wiring, connect the effect-panel widgets: `cmbEffectType::currentIndexChanged` → `handleEffectTypeChanged`; `edtEffectWidth::textEdited` → `handleEffectWidthChanged`; `edtEffectHeight::textEdited` → `handleEffectHeightChanged`; `btnEffectColor::colorChanged` → `storeEffectData` then `updateEffectImage`; `sliderOpacity::valueChanged` → `storeEffectData` then `updateEffectImage`; `edtEffectToken::textChanged` → `storeEffectData` then `updateEffectImage` (and toggle `lblTwoMinute` / `lblTwoMinuteBlank` visibility based on whether the path contains `"2-Minute Tabletop"`); `btnEffectTokenBrowse::clicked` → `selectEffectToken`; `btnTokenCW::clicked` → `handleTokenRotateCW`; `btnTokenCCW::clicked` → `handleTokenRotateCCW`; `grpShape::clicked` → `storeEffectData` then `updateEffectImage`; `btnEditConditions::clicked` → `editEffectConditions`. Set `btnEffectColor->setRotationVisible(false)` and install `QIntValidator(0, 1000, this)` on `edtEffectWidth` and `edtEffectHeight` (matches OLD constructor).
  - `setSpell(Spellv2*, bool)`: after the existing `SpellbookFactory::readObjectData(...)` call (and before `emit spellChanged()`), call `loadEffectUI()`. Guard `loadEffectUI` so it temporarily blocks the effect-control signals (e.g. via `QSignalBlocker`s on the effect widgets, or by wrapping the population in a `_loadingEffectUI` bool flag that early-returns from `storeEffectData()`); choose one mechanism and apply it consistently. The intent is that populating controls from the spell must not loop back into `storeEffectData()` and re-write the spell.
  - `loadEffectUI()`: copy the OLD `setSpell` effect-population block, translated for `Spellv2`: read `getEffectShapeActive`, `getEffectColor` (split alpha into `sliderOpacity`, RGB into `btnEffectColor->setColor`), `getEffectSize` (mirror width to height when current effect type is Radius/Cone/Cube), `getEffectToken` (and update `lblTwoMinute` / `lblTwoMinuteBlank` visibility), `getEffectType` (set `cmbEffectType` index), `getEffectTokenRotation` (into `_tokenRotation`); then call `handleEffectTypeChanged(cmbEffectType->currentIndex())` to refresh enable-states without firing `storeEffectData`, then `updateConditionLayout()` and `updateEffectImage()`. If `_spell` is null, clear all effect controls and bail.
  - `storeEffectData()`: copy the OLD `storeSpellData` effect block, translated for `Spellv2`: wrap in `_spell->beginBatchChanges()` / `endBatchChanges()`; call `setEffectType`, `setEffectShapeActive`, `setEffectSize(QSize(w, h))`, `setEffectColor(color-with-alpha)`, `setEffectToken`, `setEffectTokenRotation(_tokenRotation)`. Do not touch the name/level/school/etc. fields — those remain owned by `SpellbookFactory`. Early-return if `_spell` is null or if the load-guard flag is set.
  - `handleEffectTypeChanged(int index)`: copy OLD logic verbatim — set `lblSize` text to `"Radius"` for `BattleDialogModelEffect_Radius` else `"Size"`; enable/disable `edtEffectWidth`, `edtEffectHeight`, `grpShape`, `grpConditions` based on whether index is `BattleDialogModelEffect_Base`; then call `storeEffectData()` and `updateEffectImage()`.
  - `handleEffectWidthChanged` / `handleEffectHeightChanged`: copy OLD symmetric-mirror logic for Radius/Cone/Cube; then call `storeEffectData()` and `updateEffectImage()`.
  - `handleTokenRotateCW` / `handleTokenRotateCCW`: increment / decrement `_tokenRotation` by 90 with wrap (`>= 360 → 0`, `< 0 → 270`); then call `storeEffectData()` and `updateEffectImage()`.
  - `selectEffectToken()`: open `QFileDialog::getOpenFileName` rooted at the current token's directory if it exists; on a non-empty result, set `edtEffectToken` text (which fires the textChanged path).
  - `editEffectConditions()`: construct a stack `ConditionsEditDialog dlg`; call `dlg.setConditionList(_spell->getEffectConditionList())`; if `dlg.exec() == QDialog::Accepted`, call `_spell->setEffectConditionList(dlg.getConditionList())`, then `updateConditionLayout()`. Do not call the legacy `setConditions(int)` API.
  - `updateConditionLayout()`: clear via `clearConditionLayout()`; if `_spell` is null, return; create a fresh `QHBoxLayout` with `Qt::AlignTop | Qt::AlignHCenter`, zero margins, and a small constant spacing; assign it to `ui->frameConditions`; iterate `_spell->getEffectConditionList()` and call `addConditionIcon(id)` for each; append a stretch.
  - `addConditionIcon(const QString& id)`: resolve icon via `Conditions::activeConditions()->getConditionIconPath(id)` (skip the icon and `qDebug` a warning if `Conditions::activeConditions()` is null or path is empty); load into a `QPixmap`, scale to fit `frameConditions` height, set on a new child `QLabel`, set tooltip from `Conditions::activeConditions()->getConditionTitle(id)` or `getConditionDescription(id)`, fix the label width/height to match the icon size, and add to `_conditionLayout`. Do not use `Combatant::getConditionIcon`.
  - `clearConditionLayout()`: copy OLD `clearGrid` logic — drain and delete all items, delete the layout, null out `_conditionLayout`.
  - `updateEffectImage()`: copy OLD `updateImage` verbatim, with two adjustments: token loading uses `Spellbook::Instance()->getDirectory().filePath(...)` (unchanged); the `lblEffectImage`, `cmbEffectType`, `grpShape`, `btnEffectColor`, `sliderOpacity`, `edtEffectWidth`, `edtEffectHeight`, `edtEffectToken` accesses are all `ui->...` on this dialog. Render the parchment background, then for non-Base effect types draw the shape with current color/opacity and overlay the rotated token pixmap.
- **acceptance_criteria**:
  - `spellbooktemplatedialog.h` declares `int _tokenRotation` and `QHBoxLayout* _conditionLayout` as private members.
  - `spellbooktemplatedialog.h` declares the new private slots and helpers listed in `files_to_modify`.
  - `spellbooktemplatedialog.cpp` initialises `_tokenRotation(0)` and `_conditionLayout(nullptr)` in the constructor initialiser list.
  - The constructor contains `connect(...)` calls for every effect widget enumerated in the first integration task (effect-type combo, width/height edits, color button, opacity slider, token line-edit, token browse, token rotate CW/CCW, shape group, edit-conditions button).
  - `setSpell(Spellv2*, bool)` calls `loadEffectUI()` after the `SpellbookFactory::readObjectData` call.
  - `editEffectConditions()` calls `dlg.setConditionList(...)` and `dlg.getConditionList()`; the file contains no calls to `ConditionsEditDialog::setConditions(` or `ConditionsEditDialog::getConditions(`.
  - `editEffectConditions()` (and any other condition code) calls `_spell->getEffectConditionList()` / `_spell->setEffectConditionList(...)`; the file contains no calls to `getEffectConditions(` or `setEffectConditions(` (the legacy bitmask API).
  - `addConditionIcon` (or its inline equivalent) calls `Conditions::activeConditions()->getConditionIconPath(`; the file contains no calls to `Combatant::getConditionIcon(`.
  - `loadEffectUI()` does not cause `storeEffectData()` to write to the spell (verifiable by the presence of either `QSignalBlocker` usage on the effect widgets in `loadEffectUI` or a `_loadingEffectUI` guard checked at the top of `storeEffectData`).
  - `storeEffectData()` is invoked from each of: `handleEffectTypeChanged`, `handleEffectWidthChanged`, `handleEffectHeightChanged`, `handleTokenRotateCW`, `handleTokenRotateCCW`, the color-changed slot/lambda, the opacity-changed slot/lambda, the token-text-changed slot/lambda, and the shape-group-clicked slot/lambda.
  - `storeEffectData()` brackets its writes with `_spell->beginBatchChanges()` and `_spell->endBatchChanges()`.
  - The `TemplateFrame` / `SpellbookFactory` code path in `loadSpellUITemplate`, `postLoadConfiguration`, `getFrameObject`, `dataChanged`, and `spellRenamed` is unchanged in behaviour (no signature changes; their bodies are not modified except for the single `loadEffectUI()` call added inside `setSpell`).
  - `CMakeLists.txt` is not modified (no new translation units).
  - The full project builds with the wrapped `vcvarsall.bat` cmake command with no new errors or warnings introduced by the new code.
- **constraints_in_scope**:
  - All effect-panel logic uses the `Spellv2` `QStringList` condition API; the legacy `Spell` `int` bitmask API is forbidden in this dialog.
  - `ConditionsEditDialog` is driven via `setConditionList` / `getConditionList` only.
  - Condition icons resolve via `Conditions::activeConditions()->getConditionIconPath(id)`; do not call `Combatant::getConditionIcon`.
  - Per `cpp-qt.instructions.md`: enum values use `BattleDialogModelEffect::BattleDialogModelEffect_*` form (not `::Base` shorthand). No magic numbers — name any new scalar (e.g. condition-frame spacing, rotation step, validator max) as a `static constexpr` or `const` at the top of `spellbooktemplatedialog.cpp` rather than inlining literals.
  - Do not override any `.ui` properties (margins, spacing, stylesheets, size policies) from code beyond what the OLD reference already did programmatically (size validators, the grey-on-disabled stylesheet on size/token edits, and the runtime sizing of the conditions frame and edit-conditions button inside `showEvent` if and only if those sizes are needed — prefer to leave such cosmetic sizing to Qt Designer and call them out as `[QT DESIGNER, HUMAN]` notes if a property must change).
  - `loadEffectUI()` must not emit `dirty()` on the spell. It only reads.
- **out_of_scope**:
  - Any change to `spellbooktemplatedialog.ui` (no Qt Designer edits required — all referenced widgets already exist with the names listed in the OLD reference).
  - Any change to `Spellv2`, `Spellbook`, `SpellbookFactory`, `TemplateFrame`, `TemplateFactory`, `Conditions`, or `ConditionsEditDialog`.
  - Any change to the legacy `spellbookdialog_OLD.{h,cpp}` files.
  - Adding a Save / OK button or any new dialog field. The dialog already auto-persists on every change.
  - Any change to `CMakeLists.txt` (no new translation units).
  - Any wiring of the rolls/dice fields — the spec does not include them and the OLD reference left them as TODO.

# Cycle Log

## effect-panel-wiring

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-10
- sha_from: agent/work HEAD at dispatch
- executor_build_status: pass (ninja: no work to do — code already present in working tree)
- executor_handoff_summary: All implementation was already present in the working tree from a prior execution attempt. Build confirmed clean with no new errors.
- review_verdict: Fail (Gap)
- review_findings:
  - Medium: updateEffectImage() has unlabelled magic literals — `10`, `8` (image margin fractions) and `180` (half-rotation sentinel in the transform block) — copied verbatim from OLD without named constants, violating constraints_in_scope.
  - Info: In loadEffectUI() the width-mirror symmetry check reads cmbEffectType->currentIndex() before the spell's effect type is applied to the combo; harmless in practice but logically should use _spell->getEffectType() directly.
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-11
- executor_build_status: pass (106/106)
- executor_handoff_summary: Added named constants EFFECT_IMAGE_MARGIN_DIVISOR=10 and EFFECT_IMAGE_CONTENT_DIVISOR=8 for the updateEffectImage() fractions; expressed the half-rotation guard as EFFECT_TOKEN_ROTATION_MAX/2; changed the width-mirror check in loadEffectUI() to use _spell->getEffectType() directly. No other files touched.
- review_verdict: Pass
- review_findings:
  - Low: loadEffectUI() line ~509 uses literal `255` for shapeColor.setAlpha(255) (fully-opaque alpha); non-blocking.
- next_action: merge

# Architecture Review

# Escalations
