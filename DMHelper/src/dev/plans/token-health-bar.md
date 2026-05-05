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
status: in-progress
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
  - DMHelper/src/layertokens.h -- add `Campaign* _campaign` member (non-owning, default null), `QHash<BattleDialogModelCombatant*, BattleTokenHealthBar*> _healthBarHash`, slot `void healthBarVisibilityChanged(bool)`, helper `void refreshHealthBar(BattleDialogModelCombatant*)`, helper `void resolveCampaign()` that walks `getLayerScene()` parents to find the owning `Campaign*` and caches it in `_campaign`.
  - DMHelper/src/layertokens.cpp -- in `dmInitialize`, call `resolveCampaign()` and (when non-null) connect `Campaign::showTokenHealthBarsChanged` to `LayerTokens::healthBarVisibilityChanged`; in `addCombatant`, after creating the pixmap item, instantiate a `BattleTokenHealthBar` parented to the pixmap item and store it in the hash, then call `refreshHealthBar(combatant)`; wire HP-change signals (see integration_tasks); in `removeCombatant`, take and delete the entry; in `cleanupDM`, qDeleteAll and clear the hash and disconnect the `Campaign` connection.
  - DMHelper/src/CMakeLists.txt -- add `battletokenhealthbar.cpp` (line 102 area) and `battletokenhealthbar.h` (line 452 area) to the alphabetised source lists.
- **files_to_create**:
  - DMHelper/src/battletokenhealthbar.h -- declares `class BattleTokenHealthBar : public QGraphicsObject` (with `Q_OBJECT` macro) owning a non-owning combatant pointer, a `setVisible(bool)` based on campaign toggle, and overriding `boundingRect` and `paint`.
  - DMHelper/src/battletokenhealthbar.cpp -- implements `paint` drawing a red `QRectF` background and a green foreground sized by `RuleHealth::forCombatant(combatant)->getHealthFraction(combatant)`; `boundingRect` returns the bar rectangle in parent coordinates (full parent width, ~8% parent height, positioned just above parent y=0).
- **integration_tasks**:
  - `LayerTokens::resolveCampaign()` walks `getLayerScene()`'s `QObject::parent()` chain until it finds a `Campaign*` (use `qobject_cast`); cache the result in `_campaign`. The same member is reused by chunk 6's player-side wiring.
  - In `dmInitialize`, after `resolveCampaign()` succeeds, connect `_campaign`'s `showTokenHealthBarsChanged(bool)` to `LayerTokens::healthBarVisibilityChanged`; the slot iterates `_healthBarHash` and calls `setVisible` on each `BattleTokenHealthBar`.
  - In `LayerTokens::addCombatant`, call `refreshHealthBar(combatant)` after item creation; for monster combatants connect `BattleDialogModelMonsterBase::dataChanged` to `refreshHealthBar`; for character combatants obtain the `Characterv2*` via `BattleDialogModelCharacter::getCharacter()` and connect its inherited `CampaignObjectBase::dirty` signal to `refreshHealthBar`. Note: `CampaignObjectBase::dirty` fires on any character-sheet mutation, not only HP edits; this is acceptable because `refreshHealthBar` is cheap (recompute fraction and call `update()` on the bar).
  - `BattleTokenHealthBar::paint` must not call any GL function -- pure `QPainter` only.
  - `BattleTokenHealthBar` constructor receives the combatant pointer and the parent `QGraphicsItem*`; it does not retain ownership of the combatant and does not emit `dirty()`.
- **acceptance_criteria**:
  - `BattleTokenHealthBar` declares the `Q_OBJECT` macro and derives from `QGraphicsObject`, overriding `boundingRect` and `paint`.
  - `BattleTokenHealthBar::paint` calls `RuleHealth::forCombatant(_combatant)->getHealthFraction(_combatant)` (or returns early when null) and draws exactly two `QRectF` fills (red background, green foreground).
  - `LayerTokens` declares a `Campaign* _campaign` member and a `resolveCampaign()` helper that uses `qobject_cast` while walking `getLayerScene()->parent()` chain.
  - `LayerTokens::addCombatant` allocates one `BattleTokenHealthBar` per combatant and adds the entry to `_healthBarHash`.
  - `LayerTokens::cleanupDM` calls `qDeleteAll` and `clear()` on `_healthBarHash`.
  - `LayerTokens::healthBarVisibilityChanged` iterates the hash and toggles `setVisible` on each bar.
  - For character combatants, the diff connects `Characterv2*` (obtained via `BattleDialogModelCharacter::getCharacter()`)'s `CampaignObjectBase::dirty` signal to `refreshHealthBar`; for monster combatants, `BattleDialogModelMonsterBase::dataChanged` is connected.
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
- **dependencies**: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars, dm-token-healthbar]
- **branch**: agent/work/player-gl-token-healthbar
- **files_to_modify**:
  - DMHelper/src/publishglbattletoken.h -- add `void setHealthBarEnabled(bool campaignSetting)`, member `PublishGLTokenHighlightHealthBar* _healthBar` (non-owning observer pointer; ownership lives in `_highlightList`).
  - DMHelper/src/publishglbattletoken.cpp -- in `createTokenObjects`, when the campaign setting is true, allocate a `PublishGLTokenHighlightHealthBar(_combatant)`, append it to `_highlightList`, and cache the pointer in `_healthBar`; the existing `qDeleteAll(_highlightList)` in `cleanup` already disposes of it; `setHealthBarEnabled` adds or removes the entry from `_highlightList` (deleting on remove, allocating on add) and emits `changed()`. The constructor of `PublishGLBattleToken` is unchanged -- no GL allocation, no HP-change connect (the highlight wires its own combatant signal).
  - DMHelper/src/layertokens.cpp -- in `playerGLInitialize`, ensure `_campaign` is resolved (call `resolveCampaign()` from chunk 5 if not yet cached); for each `PublishGLBattleToken`, call `setHealthBarEnabled(_campaign && _campaign->getShowTokenHealthBars())`; connect `_campaign`'s `showTokenHealthBarsChanged(bool)` to `glHealthBarVisibilityChanged` using `Qt::QueuedConnection`. `playerGLPaint` is NOT modified -- the existing polymorphic iteration over `_highlightList` already calls `paintGL` on each highlight, and `PublishGLTokenHighlightHealthBar::paintGL` self-services its lazy-load and dirty-flag.
  - DMHelper/src/layertokens.h -- declare the new player-side slot `void glHealthBarVisibilityChanged(bool)`.
  - DMHelper/src/CMakeLists.txt -- add `publishgltokenhighlighthealthbar.cpp` (sorted near line 296) and `publishgltokenhighlighthealthbar.h` (sorted near line 648).
- **files_to_create**:
  - DMHelper/src/publishgltokenhighlighthealthbar.h -- declares `class PublishGLTokenHighlightHealthBar : public PublishGLTokenHighlight` (with `Q_OBJECT` macro) holding a non-owning `BattleDialogModelCombatant*`, an owned `PublishGLImage* _image` (default null), and a `bool _dirty` flag (default true); declares slot `void onCombatantChanged()` and helper `void rebuildPixmap()`.
  - DMHelper/src/publishgltokenhighlighthealthbar.cpp -- constructor stores the combatant pointer and connects the appropriate combatant HP-change signal (monster combatant: `BattleDialogModelMonsterBase::dataChanged`; character combatant: `BattleDialogModelCharacter::getCharacter()`-> `CampaignObjectBase::dirty`) to `onCombatantChanged()` using `Qt::QueuedConnection`; `onCombatantChanged()` only sets `_dirty = true` (no GL call); `paintGL` opens with `if(_dirty || !_image) rebuildPixmap();` then binds the `PublishGLImage` texture and draws the standard quad; `rebuildPixmap` allocates a `QImage` (e.g. 64x8) painted with red background and a green prefix of width `64 * RuleHealth::forCombatant(_combatant)->getHealthFraction(_combatant)`, replaces `_image`, and clears `_dirty`; `getWidth`/`getHeight` return the pixmap dimensions.
- **integration_tasks**:
  - All `PublishGLImage` allocation and texture upload happens inside `rebuildPixmap`, which is only ever called from `PublishGLTokenHighlightHealthBar::paintGL` (a `*GL*` function) via the lazy-load + dirty-flag guard `if(_dirty || !_image) rebuildPixmap();`. The constructor allocates no GL object.
  - The combatant HP-change signal is connected inside the `PublishGLTokenHighlightHealthBar` constructor with `Qt::QueuedConnection` to slot `onCombatantChanged()`; the slot only flips `_dirty = true` and never calls a GL function (cpp-qt.instructions.md GL context rule forbids GL calls from Qt signal handlers). `paintGL` consumes the flag at the next frame.
  - For monster combatants the highlight constructor connects `BattleDialogModelMonsterBase::dataChanged` (cast `_combatant` via `qobject_cast<BattleDialogModelMonsterBase*>`); for character combatants it connects `BattleDialogModelCharacter::getCharacter()`-> `CampaignObjectBase::dirty` (cast `_combatant` via `qobject_cast<BattleDialogModelCharacter*>`). `onCombatantChanged` is broader than HP-only for character combatants; this is acceptable because rebuilding the pixmap is cheap and only happens at next `paintGL`.
  - `PublishGLBattleToken::createTokenObjects` is the GL-context entry point that lazily creates the bar (allocation only -- the highlight itself defers GL work until its first `paintGL`). The constructor of `PublishGLBattleToken` must not allocate any GL objects.
  - `LayerTokens::playerGLInitialize` reuses the chunk-5 `_campaign` member (calling `resolveCampaign()` if not yet cached) and connects `_campaign`'s `showTokenHealthBarsChanged` to `glHealthBarVisibilityChanged` using `Qt::QueuedConnection`; the slot calls `PublishGLBattleToken::setHealthBarEnabled` on each token and emits `changed()` via the layer to trigger a repaint. `LayerTokens::playerGLPaint` is unchanged -- it iterates `_highlightList` polymorphically as before.
  - `PublishGLTokenHighlightHealthBar::setPositionScale` overrides the base to bias `_modelMatrix.translate` upward by 0.5 of the token height so the bar sits just above the token.
- **acceptance_criteria**:
  - `PublishGLTokenHighlightHealthBar` declares the `Q_OBJECT` macro and derives from `PublishGLTokenHighlight`, overriding `paintGL`, `getWidth`, `getHeight`, and `setPositionScale`.
  - `PublishGLTokenHighlightHealthBar::onCombatantChanged` slot body contains no GL call -- it only sets `_dirty = true` (verified by reading the diff).
  - `PublishGLTokenHighlightHealthBar::paintGL` opens with the lazy-load + dirty-flag guard `if(_dirty || !_image) rebuildPixmap();`.
  - `rebuildPixmap` is private/protected and is called only from `paintGL` (verified by grepping the diff for `rebuildPixmap` -- no call site outside the class itself).
  - The combatant signal connection inside `PublishGLTokenHighlightHealthBar`'s constructor uses `Qt::QueuedConnection` (verified by grepping the diff).
  - `PublishGLBattleToken::setHealthBarEnabled` adds/removes the bar from `_highlightList` and the existing `qDeleteAll(_highlightList)` cleanup is unchanged.
  - `PublishGLBattleToken` does not declare a `_healthBarDirty` member or any HP-change slot -- the dirty flag lives entirely on `PublishGLTokenHighlightHealthBar`.
  - `LayerTokens::playerGLInitialize` calls `setHealthBarEnabled` per token using the chunk-5 `_campaign` member.
  - `LayerTokens::playerGLPaint` is unmodified (no diff against the existing per-frame iteration).
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

## Chunk: ruleset-hp-keys

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:00:00Z
- branch: chunk/ruleset-hp-keys
- executor_commit_range: 44160a32..d31588df
- executor_build_status: succeeded
- executor_handoff_summary: Added four HP-key string members to `Ruleset` with getters, setters (calling `registerChange()`), defaults, and conditional XML round-trip. The pre-existing `Ruleset::internalOutputXML` did not call the base class; the plan constraint required adding that call, which causes `CampaignObjectBase::internalOutputXML` to additionally write `expanded`, `row`, and `base-icon` attributes to the `<ruleset>` element (previously omitted). The existing `inputXML` does not call the base class so those extra attributes are silently ignored on read — no regression. The `name` attribute is now written twice (by base and by existing code); the second write is a no-op. Review should confirm the new base-class attributes in the XML output are acceptable.
- review_verdict: Pass
- review_findings:
  - Info: CampaignObjectBase::internalOutputXML now writes `expanded`, `row`, and `name` onto every `<ruleset>` element; `inputXML` does not call base so all three are silently ignored on read. Harmless dead data.
  - Low: `name` is written twice (by base + pre-existing `element.setAttribute`). Idempotent, no regression; redundant line is a future cleanup candidate.
- next_action: merge

## Chunk: rulehealth-fraction

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:01:00Z
- branch: chunk/rulehealth-fraction
- executor_commit_range: bbd31da6..11edf0ac
- executor_build_status: succeeded (112/112 targets, DMHelper.exe linked cleanly)
- executor_handoff_summary: Added `getHealthFraction` virtual to `RuleHealth` base (base implementation: `clamp(getHealth/getMaxHealth, 0, 1)`, divide-by-zero guarded); added protected `currentHpKeyFor`/`maxHpKeyFor` helpers that walk the combatant's parent chain to find the campaign's ruleset and fall back to hard-coded defaults when no campaign is present; rewrote `getMaxHealth` to use `maxHpKeyFor`; overrode `getHealthFraction` in `RuleHealth5e` (count-down semantics) and `RuleHealthDaggerheart` (count-up semantics); updated `RuleHealth5e::rollInitial` to use `maxHpKeyFor` instead of the literal `"hit_points"` string. Note: commit `59de36d5` between the plan-state commit and the implementation commit contains only agent meta-file edits (made by the human); reviewers should scope their diff to the rulehealth files only.
- review_verdict: Fail
- review_findings:
  - High: `FALLBACK_MONSTER_HP_KEY` in `rulehealth.cpp` is `"hp"` but all monster XML templates store HP as `"hit_points"`. `getMaxHealth` therefore returns 0 for every monster combatant, breaking `applyDamage`/`isDead`/`isIncapacitated` for both 5e and Daggerheart. Fix: `FALLBACK_MONSTER_HP_KEY` must be `"hit_points"`. The chunk-1 ruleset defaults `RULESET_DEFAULT_MONSTER_CURRENT_HP_KEY`/`RULESET_DEFAULT_MONSTER_MAX_HP_KEY` in `ruleset.cpp` are also `"hp"` and must be corrected to `"hit_points"` as a companion fix.
  - Medium: `RuleHealth5e::getHealthFraction` is byte-for-byte identical to the base implementation — no-op override, maintenance trap. Remove it and rely on the base.
  - Medium: `currentHpKeyFor` is declared and implemented but has zero call sites in the codebase — dead protected helper. Remove it.
  - Low: `isMonster` is evaluated via `dynamic_cast` before the `if(combatant)` nullptr guard in `currentHpKeyFor`/`maxHpKeyFor`. Safe but misleading; reorder.
  - Low: `const_cast` in both key helpers to call `getParentByType` — pre-existing pattern, acceptable for now.
  - Info: No new source files, no CMakeLists change needed. ✓
  - Info: No `dirty()` emissions anywhere in the diff. ✓
  - Info: `MonsterClassv2`/`Characterv2` used correctly. ✓
  - Info: Division-by-zero guard present in all three implementations. ✓
- next_action: re-execute

### Cycle 2

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:02:00Z
- branch: chunk/rulehealth-fraction
- executor_commit_range: n/a (changes left unstaged per pipeline rules)
- executor_build_status: succeeded (112/112, DMHelper.exe linked cleanly, no new warnings)
- executor_handoff_summary: Fixed all cycle-1 review findings. `FALLBACK_MONSTER_HP_KEY` in `rulehealth.cpp` corrected from `"hp"` to `"hit_points"`; monster HP key defaults in `ruleset.cpp` likewise corrected. No-op `RuleHealth5e::getHealthFraction` override removed from header and .cpp. Unused `currentHpKeyFor` helper and its associated `FALLBACK_CHARACTER_CURRENT_HP_KEY` constant removed. `maxHpKeyFor` reordered so `if(!combatant)` null guard fires before `dynamic_cast`.
- review_verdict: Pass
- review_findings:
  - Info: Plan summary prose still says monster HP defaults are `"hp"/"hp"`; code correctly uses `"hit_points"/"hit_points"` after the fix. Stale prose, no code action needed.
  - Info: `const_cast` in `maxHpKeyFor` pre-existing pattern, Info-only.
- next_action: merge

## Chunk: monster-max-hp

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:03:00Z
- branch: chunk/monster-max-hp
- executor_commit_range: n/a (changes left unstaged per pipeline rules)
- executor_build_status: succeeded (114/114, DMHelper.exe linked cleanly, no new warnings)
- executor_handoff_summary: Added `_monsterMaxHP` (default -1) to `BattleDialogModelMonsterCombatant` with getter, setter emitting `dataChanged`, conditional XML round-trip (writes only when > 0), and `copyValues` update. `RuleHealth::getMaxHealth` now checks for a stored monster max via `dynamic_cast` before falling back to the template-key path. `RuleHealth5e::rollInitial` writes the rolled value via `setMonsterMaxHP` for monster combatants. Note: the pre-existing `internalOutputXML` called base class LAST; this was corrected to base-first per the plan constraint. Review should confirm no attribute name overlap between base and subclass levels.
- review_verdict: Pass
- review_findings:
  - Info: `-1` sentinel for `_monsterMaxHP` consistent with pre-existing `_monsterHP` pattern; no named constant needed.
- next_action: merge

## Chunk: campaign-show-health-bars

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:04:00Z
- branch: chunk/campaign-show-health-bars
- executor_commit_range: n/a
- executor_build_status: success (110/110, DMHelper.exe linked cleanly, no new warnings)
- executor_handoff_summary: All C++ changes implemented in campaign.h, campaign.cpp, optionsdialog.cpp. Human added chkShowTokenHealthBars to optionsdialog.ui in Qt Designer. Build passes cleanly after UI change. All acceptance criteria met.
- review_verdict: Pass
- review_findings: []
- next_action: merge

## Chunk: dm-token-healthbar

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:05:00Z
- branch: chunk/dm-token-healthbar
- executor_commit_range: n/a
- executor_build_status: success (no new warnings; ninja: no work to do on re-run confirming all files compiled cleanly)
- executor_handoff_summary: New files battletokenhealthbar.h and battletokenhealthbar.cpp created. layertokens.h and layertokens.cpp modified with Campaign* _campaign member, resolveCampaign(), healthBarVisibilityChanged, refreshHealthBar, and _healthBarHash. CMakeLists.txt updated with both new entries. Virtual destructor added to BattleTokenHealthBar (conventional for QGraphicsObject, not in plan spec). Health bars deleted before parent pixmap items in cleanupDM and removeCombatant.
- review_verdict: Pass
- review_findings:
  - Info: At exactly 0 HP, greenRect is empty and the green fillRect is skipped — only one fill executes instead of two. Visually correct; minor prose deviation at the 0-HP boundary only.
  - Info: Virtual destructor added to BattleTokenHealthBar — standard C++ practice, no risk.
  - Info: Health bars for combatants loaded from XML are created in the post-dmInitialize loop in dmInitialize, not inside addCombatant; HP-change signal connections established in addCombatant regardless. No functional gap.
- next_action: merge

## Chunk: player-gl-token-healthbar

### Cycle 1

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:06:00Z
- branch: chunk/player-gl-token-healthbar
- executor_commit_range: n/a
- executor_build_status: success (117/117)
- executor_handoff_summary: New files publishgltokenhighlighthealthbar.h/.cpp created. publishglbattletoken.h/.cpp, layertokens.h/.cpp, CMakeLists.txt modified. Base class paintGL signature is (QOpenGLFunctions*, int shaderModelMatrix); setPositionScale is (const QVector3D&, float) — actual codebase signatures followed. Health bars always created in createTokenObjects; the _recreateToken path will re-add a bar even when previously disabled.
- review_verdict: Fail
- review_findings:
  - High: createTokenObjects unconditionally allocates a new health bar regardless of campaign setting. The _recreateToken path (cleanup() + createTokenObjects()) re-adds the bar after any conditionsChanged event even when setHealthBarEnabled(false) was called. Fix: add bool _healthBarEnabled (default false) to PublishGLBattleToken; set it in setHealthBarEnabled; check it in createTokenObjects before allocating.
  - Low: Literal 0.5f in setPositionScale is a non-trivial constant; must be a named static constexpr.
- next_action: re-execute

### Cycle 2

- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-05T00:07:00Z
- branch: chunk/player-gl-token-healthbar
- executor_commit_range: n/a
- executor_build_status: success (111/111)
- executor_handoff_summary: Added bool _healthBarEnabled (default false) to PublishGLBattleToken header and constructor; setHealthBarEnabled now sets _healthBarEnabled before add/remove logic; createTokenObjects wraps health bar allocation in if(_healthBarEnabled). Replaced literal 0.5f in setPositionScale with named constant HEALTH_BAR_Y_BIAS.
- review_verdict: Pass
- review_findings:
  - Info: Build log not included in handoff; reviewer found no compilation hazards and recommends a cmake run to confirm before merge.
- next_action: merge

# Architecture Review

## Pre-Implementation Review -- 2026-05-04

reviewer_model: opus
verdict: Revise
summary: The plan is architecturally sound in its overall direction (clean separation of DM and player layer paths, ruleset-configurable HP keys with defensible defaults, serialization shape changes that respect the base-class rule and avoid `dirty()` from `inputXML`/constructors). Three issues require correction before execution: chunk 6's wording for the combatant-to-renderer slot permits a direct GL call from a Qt signal handler, which violates the GL-context rule; chunk 6 references `_campaign->getShowTokenHealthBars()` as if `LayerTokens` already stored a `Campaign*`, which it does not; and chunk 5's character-HP-change wiring names a non-existent `Combatant::dirty` signal source.

triggers_evaluated:
  - threading: concern: chunk 6's slot path allows a direct call to `rebuildPixmap` (a GL-using function) from a Qt signal handler under a `QOpenGLContext::currentContext()` guard, which is not a valid substitute for being inside a `*GL*` function (cpp-qt.instructions.md GL context rule explicitly bans GL calls from "Qt signal handlers").
  - layer_interface: addressed (chunk 5 implements DM-only `dmInitialize`/`QGraphicsObject` path; chunk 6 implements player-only `playerGLInitialize`/`paintGL` path; neither path assumes the other is active).
  - serialization_shape: addressed (chunks 1, 3, 4 each name the base-class call in `internalOutputXML`; no `dirty()` emission in any `inputXML`/constructor; no cross-references introduced so `postProcessXML` is not needed).
  - subsystem_boundary: addressed (battle, campaign, UI shell coupling matches the risk assessment; the `[QT DESIGNER, HUMAN]` integration task correctly delegates `.ui` work).
  - new_subsystem_or_flag: not-applicable (no new `dmconstants.h` flag, no new top-level subsystem; `INCLUDE_NETWORK_SUPPORT` and `LAYERVIDEO_USE_OPENGL` untouched).

findings:
  - High: Chunk 6 integration_tasks -- the sentence "the slot guards with `if(QOpenGLContext::currentContext())` before calling `rebuildPixmap`; otherwise it sets a `_dirtyPixmap` flag" permits a GL call from a queued slot handler. cpp-qt.instructions.md forbids GL calls from any function not named `*GL*`, including signal handlers. The slot must always defer to `paintGL` via `_dirtyPixmap`; the context-current branch must be removed.
  - Medium: Chunk 6 files_to_modify (layertokens.cpp) -- references `_campaign->getShowTokenHealthBars()` as if `LayerTokens` already stored a `Campaign*`. `LayerTokens`'s constructor takes only a `BattleDialogModel*`; chunk 5 itself acquires Campaign on demand via `getLayerScene()->getModel()`'s parent chain. Either chunk 6 must use the same on-demand acquisition pattern, or one of the two chunks must explicitly add a `Campaign*` member to `LayerTokens` and the dependency must be reflected in both chunks.
  - Medium: Chunk 5 integration_tasks -- "connect `Combatant::dirty` for character combatants (the underlying `Characterv2` accessed via `BattleDialogModelCharacter::getCharacter()`)" mixes two different signal sources. `Combatant` (the PC base class) has an empty `signals:` block; the actual signal that fires when the character sheet's HP changes is `CampaignObjectBase::dirty()` inherited by `Characterv2`. Wording should be: connect `BattleDialogModelCharacter::getCharacter()` (a `Characterv2*`)'s `CampaignObjectBase::dirty` signal to `refreshHealthBar`, and acknowledge that this fires on any character mutation, not only HP edits.
  - Low: Chunk 4 -- spec success criterion 7 ("Changing the HP field names in the ruleset causes the health bar to read the correct fields") implies a UI for the four ruleset HP keys, but the plan exposes them only via XML round-trip. This matches the explicit out_of_scope ("hard-coded defaults inside `Ruleset` for now"); flagging only because spec authors may consider XML-only configuration insufficient.
  - Low: Chunk 6 -- `BattleTokenHealthBar` and `PublishGLTokenHighlightHealthBar` are both `QObject`-derived but the `Q_OBJECT` macro requirement is implicit. Worth naming explicitly in `files_to_create` to avoid an Execution-time MOC oversight.
  - Info: Chunk 5 acceptance criterion "draws exactly two `QRectF` fills" is precise and verifiable from diff -- good.

required_plan_changes:
  - Rewrite chunk 6 integration_tasks bullet on the queued slot so that the slot only sets `_dirtyPixmap` and emits `changed()`; remove the `QOpenGLContext::currentContext()`-guarded direct call to `rebuildPixmap`. State that `paintGL` consumes the flag at its top via the lazy-load guard pattern.
  - Reconcile the `Campaign` acquisition strategy across chunks 5 and 6: either (a) add a `Campaign*` member to `LayerTokens` populated in `dmInitialize`/`playerGLInitialize` from `getLayerScene()`'s parent chain, naming this in both chunks' files_to_modify, or (b) replace chunk 6's `_campaign->getShowTokenHealthBars()` references with the same on-demand acquisition pattern used in chunk 5's `dmInitialize` connect.
  - Rewrite chunk 5's character-HP wiring bullet to name `BattleDialogModelCharacter::getCharacter()` returning a `Characterv2*` and connect to its inherited `CampaignObjectBase::dirty` signal; note explicitly that this fires on any character mutation and is broader than HP-only.
  - Add `Q_OBJECT` macro requirement to the `files_to_create` description for both `battletokenhealthbar.h` and `publishgltokenhighlighthealthbar.h`.

## Pre-Implementation Review -- 2026-05-05

reviewer_model: opus
verdict: Revise
summary: Three of the four required plan changes from the prior review are correctly applied: chunk 6's queued slot now defers to `playerGLPaint` via `_healthBarDirty` with explicit "must NOT call any GL function" wording; chunk 5's character-HP wiring now names `BattleDialogModelCharacter::getCharacter()` returning `Characterv2*` and `CampaignObjectBase::dirty` with the broader-than-HP caveat; both new headers explicitly require the `Q_OBJECT` macro. The fourth change (Campaign-acquisition reconciliation) chose option (a) -- a single `_campaign` member on `LayerTokens` populated by `resolveCampaign()` and shared by both chunks -- but chunk 6's `dependencies` field (line 221) was not updated to include `dm-token-healthbar`, even though chunk 6 now reads the `_campaign` member and calls `resolveCampaign()` introduced in chunk 5.

triggers_evaluated:
  - threading: addressed (chunk 6 integration_tasks and acceptance criterion both pin `refreshHealthBar` to `_healthBarDirty` mutation + `emit changed()` only; queued connection preserved; `playerGLPaint` consumes the flag while the GL context is current; no `QOpenGLContext::currentContext()` shortcut remains).
  - layer_interface: addressed (chunk 5 stays on DM `QGraphicsObject`/`dmInitialize`; chunk 6 stays on player `playerGLInitialize`/`paintGL`; the shared `_campaign` member is non-owning data, not behavioural cross-coupling).
  - serialization_shape: addressed (unchanged from prior review; chunks 1, 3, 4 still call the base class in `internalOutputXML` and avoid `dirty()` from `inputXML`/constructors).
  - subsystem_boundary: concern: chunk 6 modifies `layertokens.cpp` to call `resolveCampaign()` and read the `_campaign` member added by chunk 5, but chunk 6's `dependencies: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars]` does not list `dm-token-healthbar`. Execution may dispatch chunk 6 before chunk 5 and the `_campaign` reference will not compile.
  - new_subsystem_or_flag: not-applicable (unchanged).

findings:
  - Medium: Chunk 6 dependencies (line 221) -- omits `dm-token-healthbar`. Chunk 6's `files_to_modify` for `layertokens.cpp` and its integration_tasks both reference "the chunk-5 `_campaign` member" and "`resolveCampaign()` from chunk 5"; without the dependency edge, Execution can pick chunk 6 before the member exists in `layertokens.h`.
  - Info: Required change #1 is reinforced in both integration_tasks and acceptance_criteria -- good defence-in-depth against the GL-from-slot violation that drove the prior Revise.
  - Info: Chunk 6 `setPositionScale` bias is described as 0.5 of token height; vertical placement is cosmetic, not architectural, and need not be pinned in acceptance criteria.

required_plan_changes:
  - In chunk 6, change `dependencies: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars]` to `dependencies: [rulehealth-fraction, monster-max-hp, campaign-show-health-bars, dm-token-healthbar]` so Execution cannot pick chunk 6 before chunk 5 has added `_campaign` and `resolveCampaign()` to `LayerTokens`.

## Pre-Implementation Review -- 2026-05-05

reviewer_model: opus
verdict: Pass
summary: All required plan changes from the prior two reviews are now applied. Chunk 6's dependency list at line 221 reads `[rulehealth-fraction, monster-max-hp, campaign-show-health-bars, dm-token-healthbar]`, closing the cross-chunk integration-order hole; the queued slot defers to `playerGLPaint` via `_healthBarDirty` with no GL call in the slot body; chunk 5's character-HP wiring correctly names `BattleDialogModelCharacter::getCharacter()` and `CampaignObjectBase::dirty` with the broader-than-HP caveat; both new headers explicitly require `Q_OBJECT`. Execution may proceed.

triggers_evaluated:
  - threading: addressed (queued connection preserved; `refreshHealthBar` slot body is `_healthBarDirty = true; emit changed();` only; `playerGLPaint` consumes the flag and calls `rebuildPixmap()` while the GL context is current; no `QOpenGLContext::currentContext()` shortcut anywhere).
  - layer_interface: addressed (chunk 5 owns the DM `QGraphicsObject`/`dmInitialize` path; chunk 6 owns the player `playerGLInitialize`/`paintGL` path; the shared `_campaign` member is non-owning data populated independently on each path via `resolveCampaign()`).
  - serialization_shape: addressed (chunks 1, 3, 4 each call the base-class `internalOutputXML`; no `dirty()` emission in any new constructor or `inputXML`; no cross-references requiring `postProcessXML`).
  - subsystem_boundary: addressed (chunk 6 now declares its dependency on `dm-token-healthbar`, so the `_campaign` member and `resolveCampaign()` helper are guaranteed to be in the tree before chunk 6's `layertokens.cpp` edits land; battle/campaign/UI shell coupling matches the risk assessment; `[QT DESIGNER, HUMAN]` integration task correctly delegates `.ui` work).
  - new_subsystem_or_flag: not-applicable (no new `dmconstants.h` flag, no new top-level subsystem; `INCLUDE_NETWORK_SUPPORT` and `LAYERVIDEO_USE_OPENGL` untouched).

findings: []

# Escalations