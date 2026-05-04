---
feature_slug: token-health-bar
spec_path: DMHelper/src/dev/specs/token-health-bar.md
created: 2026-05-04
designer_model: opus
arch_review_required: true
arch_review_model: opus
arch_review_reason: "Touches GL render path on the player screen, modifies the LayerTokens layer subclass on the DM side, and changes serialization shape on Campaign/Ruleset/BattleDialogModelMonsterCombatant."
pre_impl_arch_review_requested: true
supersedes: null
status: approved
---

# Summary

Adds an opt-in per-combatant health bar overlay rendered above each token on
both the DM `QGraphicsScene` view and the player-facing OpenGL screen. The
toggle lives on `Campaign` and is exposed in the existing `OptionsDialog`
campaign tab; default is off and the setting is XML-serialized with the
campaign root element.

The HP read/write path is reorganised so that the property names used to fetch
current and maximum HP are configurable per ruleset (stored as four string
attributes on `Ruleset`, with defaults `hit_points`/`maximumHp` for characters
and `hp`/`hp` for monsters). `RuleHealth` gains a `getHealthFraction()`
[0,1] accessor; concrete subclasses interpret the fraction according to
count-down (5e) vs count-up (Daggerheart) semantics. Character combatant HP
remains live-linked through the existing `BattleDialogModelCharacter`
delegation to `Characterv2`; monster combatants gain a serialized
per-instance `_monsterMaxHP` set at `rollInitial` time.

Subsystems touched: `battle` (combatant model, layer, GL token), `campaign`
(setting + serialization), `UI shell` (`OptionsDialog` campaign tab Qt
Designer change). All `.ui` and `.qrc` work is delegated to the human via
`[QT DESIGNER, HUMAN]` integration tasks.

# Architectural Risk Assessment

- Trigger 1 (threading / GL boundary): **Hit** -- adds a new GL highlight type
  rendered through the existing `PublishGLBattleToken`/`PublishGLTokenHighlight`
  path in `LayerTokens::playerGLPaint`; texture upload of the bar pixmap must
  obey the GL-context rule.
- Trigger 2 (Layer subclass change): **Hit** -- `LayerTokens` is modified on
  both DM (`dmInitialize` / per-combatant graphics-item creation) and player
  (`playerGLPaint`) sides to manage health-bar overlays.
- Trigger 3 (serialization shape change): **Hit** -- adds attributes to
  `<campaign>`, `<ruleset>`, and the monster-combatant element.
- Trigger 4 (multi-subsystem): **Hit** -- battle + campaign + UI shell.
- Trigger 5 (new top-level subsystem / new `dmconstants.h` flag): **Not hit**.

`arch_review_required = true`, `arch_review_model = opus`,
`pre_impl_arch_review_requested = true`.

# Chunks

## Chunk 1: ruleset-hp-keys

- **id**: ruleset-hp-keys
- **summary**: Add four configurable HP-field-name strings to `Ruleset`
  (character current/max, monster current/max) with XML round-trip and defaults.
- **dependencies**: []
- **branch**: agent/work/ruleset-hp-keys
- **files_to_modify**:
  - DMHelper/src/ruleset.h -- declare `_characterCurrentHpKey`, `_characterMaxHpKey`, `_monsterCurrentHpKey`, `_monsterMaxHpKey` plus four getter/setter pairs.
  - DMHelper/src/ruleset.cpp -- initialise defaults (`hit_points`, `maximumHp`, `hp`, `hp`), serialize/deserialize via `internalOutputXML` and `inputXML`, call `registerChange()` on setters.
- **files_to_create**: []
- **integration_tasks**:
  - In `Ruleset::internalOutputXML` write the four keys as attributes `characterCurrentHpKey`, `characterMaxHpKey`, `monsterCurrentHpKey`, `monsterMaxHpKey` only when their value differs from the default.
  - In `Ruleset::inputXML` read each attribute with the corresponding default as fallback before existing logic returns.
- **acceptance_criteria**:
  - `Ruleset` exposes `getCharacterCurrentHpKey()`, `getCharacterMaxHpKey()`, `getMonsterCurrentHpKey()`, `getMonsterMaxHpKey()` and matching setters.
  - Round-trip XML preserves any non-default value on each of the four keys (verified by reading the diff in `ruleset.cpp`).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Always call the base class implementation in `internalOutputXML` (cpp-qt.instructions.md serialisation rule).
  - Setters must emit `rulesetChanged()` via `registerChange()`, mirroring existing string setters.
- **out_of_scope**:
  - Changing any current `RuleHealth` behaviour.
  - Adding ruleset-template defaults; the four strings are hard-coded defaults inside `Ruleset` for now.

## Chunk 2: rulehealth-fraction

- **id**: rulehealth-fraction
- **summary**: Add `RuleHealth::getHealthFraction(combatant)` returning [0,1]
  and route `getMaxHealth` / `rollInitial` through the ruleset-configured keys
  added in chunk 1.
- **dependencies**: [ruleset-hp-keys]
- **branch**: agent/work/rulehealth-fraction
- **files_to_modify**:
  - DMHelper/src/rulehealth.h -- declare new virtual `qreal getHealthFraction(const BattleDialogModelCombatant*) const`; add protected helpers `currentHpKeyFor(combatant)` / `maxHpKeyFor(combatant)`.
  - DMHelper/src/rulehealth.cpp -- implement the helpers (resolve via `forCombatant(combatant)->getRuleset` or local equivalent); rewrite `getMaxHealth` to use `maxHpKeyFor`; provide a base `getHealthFraction` that returns `clamp(getHealth/getMaxHealth, 0, 1)`.
  - DMHelper/src/rulehealth5e.h -- declare override of `getHealthFraction`.
  - DMHelper/src/rulehealth5e.cpp -- override `getHealthFraction` with count-down semantics (`current/max`, clamped); update `rollInitial` to read from `currentHpKeyFor`/`maxHpKeyFor` instead of literal `"hit_points"`/`"hit_dice"` (keep `"hit_dice"` as the dice key -- only HP keys move to ruleset config).
  - DMHelper/src/rulehealthdaggerheart.h -- declare override of `getHealthFraction`.
  - DMHelper/src/rulehealthdaggerheart.cpp -- override `getHealthFraction` with count-up semantics (`(max - current)/max` -- fully green at 0, fully red at max).
- **files_to_create**: []
- **integration_tasks**:
  - In `RuleHealth::templateFor`-using helpers, fetch the ruleset via `forCombatant(combatant)->getRuleset()` (use `Campaign* campaign = ...; campaign->getRuleset()`) -- do not pass nullptr through.
  - When ruleset lookup yields nullptr, fall back to literal `"hit_points"`/`"maximumHp"`/`"hp"` defaults so unit-test combatants without a campaign continue working.
  - Guard division by zero in `getHealthFraction` -- return 0.0 when max <= 0.
- **acceptance_criteria**:
  - `RuleHealth` declares and defines `getHealthFraction`.
  - `RuleHealth5e::rollInitial` no longer references the literal string `"hit_points"` directly; it calls a key-resolution helper.
  - `RuleHealth5e::getHealthFraction` returns 1.0 when `getHealth == getMaxHealth` and 0.0 when `getHealth <= 0`.
  - `RuleHealthDaggerheart::getHealthFraction` returns 1.0 when `getHealth == 0` and 0.0 when `getHealth >= getMaxHealth`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Use `MonsterClassv2`/`Characterv2` (v2 only) when reading from the template object.
  - Do not emit `dirty()` from RuleHealth -- these are read-only paths.
- **out_of_scope**:
  - Storing the fraction anywhere; it is computed on demand.
  - Wiring into render paths (chunks 5 and 6).

## Chunk 3: monster-max-hp

- **id**: monster-max-hp
- **summary**: Add a serialized `_monsterMaxHP` field to
  `BattleDialogModelMonsterCombatant`, set it at `rollInitial` time, and have
  `RuleHealth::getMaxHealth` prefer the stored monster max for monster
  combatants.
- **dependencies**: [rulehealth-fraction]
- **branch**: agent/work/monster-max-hp
- **files_to_modify**:
  - DMHelper/src/battledialogmodelmonstercombatant.h -- add `int _monsterMaxHP` member, `int getMonsterMaxHP() const`, `void setMonsterMaxHP(int)`.
  - DMHelper/src/battledialogmodelmonstercombatant.cpp -- initialise `_monsterMaxHP` to `-1` in all constructors, read/write attribute `monsterMaxHP` in `inputXML`/`internalOutputXML`, copy it in `copyValues`, emit `dataChanged(this)` from the setter when the value changes.
  - DMHelper/src/rulehealth.cpp -- in `getMaxHealth`, when the combatant is a `BattleDialogModelMonsterCombatant` with `getMonsterMaxHP() > 0` return that; otherwise fall back to the chunk-2 template-key path.
  - DMHelper/src/rulehealth5e.cpp -- in `rollInitial`, after computing the rolled value, call `monsterCombatant->setMonsterMaxHP(rolled)` when the combatant is a `BattleDialogModelMonsterCombatant` (use `dynamic_cast`).
- **files_to_create**: []
- **integration_tasks**:
  - `internalOutputXML` writes `monsterMaxHP` only when `_monsterMaxHP > 0` to keep diffs minimal on legacy campaigns.
  - `inputXML` reads `element.attribute("monsterMaxHP", QString::number(-1)).toInt()` so absent attribute leaves the field unresolved and the template fallback kicks in.
  - `setMonsterMaxHP` must call the base class change-tracking mechanism used by other setters in this file (it already calls `emit dataChanged(this)`); do not emit `dirty()`.
- **acceptance_criteria**:
  - `BattleDialogModelMonsterCombatant` declares `_monsterMaxHP`, `getMonsterMaxHP`, `setMonsterMaxHP`.
  - `internalOutputXML` calls the base class first and conditionally writes `monsterMaxHP`.
  - `RuleHealth::getMaxHealth` contains a `dynamic_cast<const BattleDialogModelMonsterCombatant*>` branch returning the stored max when positive.
  - `RuleHealth5e::rollInitial` writes the rolled max via `setMonsterMaxHP` for monster combatants.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Always call the base class `internalOutputXML` first (already done in this file; do not regress).
  - Setter emits `dataChanged`, not `dirty()`, in keeping with existing `setHitPoints` pattern.
- **out_of_scope**:
  - Modifying `MonsterClassv2`.
  - Persisting character HP on the combatant.

## Chunk 4: campaign-show-health-bars

- **id**: campaign-show-health-bars
- **summary**: Add a `bool _showTokenHealthBars` to `Campaign` with serialization
  and an OptionsDialog campaign-tab checkbox.
- **dependencies**: []
- **branch**: agent/work/campaign-show-health-bars
- **files_to_modify**:
  - DMHelper/src/campaign.h -- declare `_showTokenHealthBars` (default false), `bool getShowTokenHealthBars() const`, public slot `void setShowTokenHealthBars(bool)`, signal `void showTokenHealthBarsChanged(bool)`.
  - DMHelper/src/campaign.cpp -- initialise to `false` in constructor, read in `inputXML` (`element.attribute("showTokenHealthBars", QString::number(0)).toInt() != 0`), write in `internalOutputXML` only when true (mirrors `_lastMonster`/`_fearCount` pattern), implement setter that emits `showTokenHealthBarsChanged` and `registerChange`/equivalent mark-dirty path used by `setFearCount`.
  - DMHelper/src/optionsdialog.h -- add `void showTokenHealthBarsChanged(bool)` private slot only if the new checkbox cannot be wired by name via `auto-connection`; otherwise no header change.
  - DMHelper/src/optionsdialog.cpp -- read `_campaign->getShowTokenHealthBars()` into the new checkbox in the campaign-tab populate block (around line 93), write back via `_campaign->setShowTokenHealthBars(ui->chkShowTokenHealthBars->isChecked())` in the same accept/save path other campaign options use.
- **files_to_create**: []
- **integration_tasks**:
  - `[QT DESIGNER, HUMAN]` In `optionsdialog.ui`, on the campaign tab (the tab containing `chkCombatantDone`), add a `QCheckBox` named `chkShowTokenHealthBars` with text "Show token health bars" placed directly below `chkCombatantDone` in the same vertical layout.
  - In `Campaign::setShowTokenHealthBars`, use the same dirty-emission pattern as `setFearCount` (which calls into `handleInternalDirty`) -- do not emit `dirty()` from any constructor or `inputXML`.
  - In `OptionsDialog`, locate the existing block that copies campaign-tab settings into the `Campaign`/`Ruleset` on accept and add the `setShowTokenHealthBars` call alongside it; do not introduce new save plumbing.
- **acceptance_criteria**:
  - `Campaign` declares `_showTokenHealthBars`, `getShowTokenHealthBars`, `setShowTokenHealthBars`, `showTokenHealthBarsChanged`.
  - `Campaign::internalOutputXML` writes attribute `showTokenHealthBars` conditionally on its true value and calls the base class.
  - `Campaign::inputXML` reads `showTokenHealthBars` attribute with `0` default before doing nothing else with it (no `dirty()` emission inside `inputXML`).
  - `OptionsDialog` references `ui->chkShowTokenHealthBars` in both the populate path (around the `chkCombatantDone` line) and the accept path.
  - Build succeeds with no new warnings (the checkbox must already exist in the .ui -- execution will block on the human-mediated step).
- **constraints_in_scope**:
  - No code modification of `optionsdialog.ui`; checkbox addition is a Qt Designer step.
  - No `dirty()` emission from `inputXML`; setter follows the `setFearCount` pattern for change notification.
- **out_of_scope**:
  - Persisting setting outside `<campaign>` element.
  - Adding any per-campaign options container; the boolean lives directly on `Campaign`.

## Chunk 5: dm-token-healthbar

- **id**: dm-token-healthbar
- **summary**: Render the DM-side health bar as a child `QGraphicsItem` of each
  combatant pixmap in `LayerTokens`, driven by the campaign setting and the
  `RuleHealth` fraction.
- **dependencies**: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars]
- **branch**: agent/work/dm-token-healthbar
- **files_to_modify**:
  - DMHelper/src/layertokens.h -- add `QHash<BattleDialogModelCombatant*, BattleTokenHealthBar*> _healthBarHash`, slot `void healthBarVisibilityChanged(bool)`, helper `void refreshHealthBar(BattleDialogModelCombatant*)`.
  - DMHelper/src/layertokens.cpp -- in `addCombatant`, after creating the pixmap item, instantiate a `BattleTokenHealthBar` parented to the pixmap item and store it in the hash; in `removeCombatant`, take and delete the entry; in `cleanupDM`, qDeleteAll the hash; in the `addCombatant` connect block, hook `BattleDialogModelMonsterBase::dataChanged` and `BattleDialogModelCharacter`-equivalent change source to `refreshHealthBar`; honour the campaign setting via the new slot connected to `Campaign::showTokenHealthBarsChanged`.
  - DMHelper/src/CMakeLists.txt -- add `battletokenhealthbar.cpp` (line 102 area) and `battletokenhealthbar.h` (line 452 area) to the alphabetised source lists.
- **files_to_create**:
  - DMHelper/src/battletokenhealthbar.h -- declares `class BattleTokenHealthBar : public QGraphicsObject` owning a non-owning combatant pointer, a `setVisible(bool)` based on campaign toggle, and overriding `boundingRect` and `paint`.
  - DMHelper/src/battletokenhealthbar.cpp -- implements `paint` drawing a red `QRectF` background and a green foreground sized by `RuleHealth::forCombatant(combatant)->getHealthFraction(combatant)`; `boundingRect` returns the bar rectangle in parent coordinates (full parent width, ~8% parent height, positioned just above parent y=0).
- **integration_tasks**:
  - Connect `Campaign::showTokenHealthBarsChanged` (acquired via `getLayerScene()->getModel()`'s parent chain to `Campaign`, mirroring `RuleHealth::forCombatant`'s lookup) to `LayerTokens::healthBarVisibilityChanged` once during `dmInitialize`; on signal, iterate the hash and call `setVisible` on each `BattleTokenHealthBar`.
  - In `LayerTokens::addCombatant`, call `refreshHealthBar(combatant)` after item creation and connect existing `BattleDialogModelMonsterBase::dataChanged` (for monster combatants) and `Combatant::dirty` for character combatants (the underlying `Characterv2` accessed via `BattleDialogModelCharacter::getCharacter()`) to `refreshHealthBar`.
  - `BattleTokenHealthBar::paint` must not call any GL function -- pure `QPainter` only.
  - `BattleTokenHealthBar` constructor receives the combatant pointer and the parent `QGraphicsItem*`; it does not retain ownership of the combatant and does not emit `dirty()`.
- **acceptance_criteria**:
  - `BattleTokenHealthBar` derives from `QGraphicsObject` and overrides `boundingRect` and `paint`.
  - `BattleTokenHealthBar::paint` calls `RuleHealth::forCombatant(_combatant)->getHealthFraction(_combatant)` (or returns early when null) and draws exactly two `QRectF` fills (red background, green foreground).
  - `LayerTokens::addCombatant` allocates one `BattleTokenHealthBar` per combatant and adds the entry to `_healthBarHash`.
  - `LayerTokens::cleanupDM` calls `qDeleteAll` and `clear()` on `_healthBarHash`.
  - `LayerTokens::healthBarVisibilityChanged` iterates the hash and toggles `setVisible` on each bar.
  - CMakeLists.txt contains both the .cpp and .h entries.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - No GL calls -- DM path is `QPainter` only (cpp-qt.instructions.md GL context rule).
  - `Layer` subclass path: DM-only methods may not assume GL state.
  - Do not override `.ui` properties from code; this chunk has no `.ui` work.
- **out_of_scope**:
  - Animated transitions, numeric labels, configurable colours.
  - Player-screen rendering (chunk 6).

## Chunk 6: player-gl-token-healthbar

- **id**: player-gl-token-healthbar
- **summary**: Render the player-screen health bar as a new
  `PublishGLTokenHighlight` subclass attached to each `PublishGLBattleToken`,
  controlled by the campaign setting.
- **dependencies**: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars]
- **branch**: agent/work/player-gl-token-healthbar
- **files_to_modify**:
  - DMHelper/src/publishglbattletoken.h -- add `void setHealthBarEnabled(bool)`, slot `void refreshHealthBar()`, member `PublishGLTokenHighlightHealthBar* _healthBar` (owned, lives inside `_highlightList`).
  - DMHelper/src/publishglbattletoken.cpp -- in the constructor, connect combatant `dataChanged`/HP-change signal (whichever the chunk-5 wiring identified) to `refreshHealthBar` using `Qt::QueuedConnection`; in `createTokenObjects`, allocate the health-bar highlight only when the campaign setting is true, append to `_highlightList`; in `cleanup`, the existing `qDeleteAll(_highlightList)` already disposes of it; `setHealthBarEnabled` adds or removes the entry and calls `emit changed()`.
  - DMHelper/src/layertokens.cpp -- in `playerGLInitialize`, after each `PublishGLBattleToken` is created, call `setHealthBarEnabled(_campaign->getShowTokenHealthBars())`; connect `Campaign::showTokenHealthBarsChanged` to a slot that walks all tokens and calls `setHealthBarEnabled`.
  - DMHelper/src/layertokens.h -- declare the new player-side slot `void glHealthBarVisibilityChanged(bool)`.
  - DMHelper/src/CMakeLists.txt -- add `publishgltokenhighlighthealthbar.cpp` (sorted near line 296) and `publishgltokenhighlighthealthbar.h` (sorted near line 648).
- **files_to_create**:
  - DMHelper/src/publishgltokenhighlighthealthbar.h -- declares `class PublishGLTokenHighlightHealthBar : public PublishGLTokenHighlight` owning a `PublishGLImage*`, a non-owning combatant pointer, and exposing `void rebuildPixmap()`.
  - DMHelper/src/publishgltokenhighlighthealthbar.cpp -- implements `paintGL` by binding the owned `PublishGLImage` texture and drawing the standard quad; `rebuildPixmap` (called from `playerGLInitialize` and on HP change via a queued slot) generates a small `QImage` (e.g. 64x8) painted with red background and a green prefix of width `64 * getHealthFraction(combatant)`, then constructs/replaces the `PublishGLImage`; `getWidth`/`getHeight` return the pixmap dimensions.
- **integration_tasks**:
  - All `PublishGLImage` allocation and texture upload happens inside `rebuildPixmap`, which must only be called from a `*GL*` function (`playerGLInitialize`, `playerGLPaint`, or via `QMetaObject::invokeMethod(this, ..., Qt::QueuedConnection)` so the renderer thread re-enters with the GL context current). The combatant `dataChanged` connection uses `Qt::QueuedConnection` and the slot guards with `if(QOpenGLContext::currentContext())` before calling `rebuildPixmap`; otherwise it sets a `_dirtyPixmap` flag consumed at the top of `paintGL` (lazy-load guard pattern -- see cpp-qt.instructions.md GL context rule).
  - `PublishGLBattleToken::createTokenObjects` is the GL-context entry point that lazily creates the bar; the constructor must not allocate any GL objects.
  - `LayerTokens::playerGLInitialize` connects `Campaign::showTokenHealthBarsChanged` to `glHealthBarVisibilityChanged` using `Qt::QueuedConnection`; the slot calls `PublishGLBattleToken::setHealthBarEnabled` on each token and calls `emit changed()` via the layer to trigger a repaint.
  - `PublishGLTokenHighlightHealthBar::setPositionScale` overrides the base to bias `_modelMatrix.translate` upward by 0.5 of the token height so the bar sits just above the token.
- **acceptance_criteria**:
  - `PublishGLTokenHighlightHealthBar` derives from `PublishGLTokenHighlight` and overrides `paintGL`, `getWidth`, `getHeight`.
  - All GL-resource allocation paths in the new class are reached only from functions whose names contain `GL` (verified by reading the diff).
  - `PublishGLBattleToken::setHealthBarEnabled` adds/removes the bar from `_highlightList` and the existing `qDeleteAll(_highlightList)` cleanup is unchanged.
  - `LayerTokens::playerGLInitialize` calls `setHealthBarEnabled` per token using the campaign value.
  - The combatant-to-health-bar slot connection uses `Qt::QueuedConnection` (verified by grepping the diff for `Qt::QueuedConnection`).
  - CMakeLists.txt contains both new file entries.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - GL context rule: shader/texture creation only from `*GL*` functions; lazy-load guard inside `paintGL` (cpp-qt.instructions.md).
  - Threading: combatant signals may arrive on the model thread; marshal to renderer via `Qt::QueuedConnection` (cpp-qt.instructions.md threading section).
  - `Layer` subclass path: player-side methods must not assume DM scene state.
- **out_of_scope**:
  - Custom shader authoring; reuse the existing textured-quad shader path used by `PublishGLTokenHighlightRef`.
  - Animated transitions, numeric labels, configurable colours.

# Cycle Log

# Architecture Review

# Escalations