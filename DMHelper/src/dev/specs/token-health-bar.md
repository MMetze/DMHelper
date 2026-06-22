# Spec: Token Health Bar

**Feature slug:** `token-health-bar`  
**Subsystem:** Battle / Tokens (GL player screen + DM view)  
**Status:** draft — awaiting human approval

---

## User-Visible Behavior

A small, horizontal health bar is rendered above each combatant token on the
player-facing OpenGL battle screen. The bar has a red background with a green
foreground that shrinks proportionally as the combatant loses hit points.
At full health the bar is fully green; at 0 HP it is fully red.

The health bar is **disabled by default** and is toggled per-campaign via a
campaign settings option.

---

## Feature Requirements

### 1. Campaign Setting

- A boolean setting "Show token health bars" is added to the campaign settings,
  stored on `Campaign` (or its options container), default `false`.
- The setting is serialized and persisted with the campaign file.

### 2. Rendering (Player GL Screen and DM View)

- When the setting is `true`, each combatant token shown in **both** the
  player-facing OpenGL battle screen (`PublishGLBattleToken` or equivalent)
  and the **DM battle scene view** (graphics-item layer) shows a rectangular
  health bar overlay above the token image.
- Bar dimensions: proportional to the token's rendered size (e.g. full token
  width, ~8% of token height).
- Bar position: immediately above the token image, centered horizontally.
- Visual: red background, green foreground scaled by `currentHP / maxHP`.
  When `currentHP <= 0`, bar is entirely red.
- On the player GL screen the bar is drawn using the existing OpenGL
  infrastructure (quads / shader primitives already used for token highlights
  and overlays).
- On the DM view the bar is drawn using Qt's `QPainter` API within the
  relevant `QGraphicsItem` paint method.

### 3. Health Data per Combatant

#### Field Name Configuration (Ruleset)

- The property names used to read current HP and maximum HP are **not
  hard-coded**. They are defined in the active ruleset (e.g. `RuleHealth` or
  the ruleset settings) so that different rulesets can use different field
  names.
- Default values (used when no ruleset override is present):
  - Characters: current HP → `hit_points`; maximum HP → `maximumHp`.
  - Monsters: current HP → `hp`; maximum HP → `hp` (at creation; see below).
- The ruleset configuration for these field names is serialized with the
  ruleset, not with individual combatants.

#### Characters (PCs and NPCs — `BattleDialogModelCharacter`)

- Character combatant HP is **live-linked** to the underlying `Characterv2`
  object via the ruleset-configured field names.
- Changes made on the character sheet are immediately reflected in the battle
  combatant, and changes made in the battle (e.g. applying damage) are written
  back to the character sheet.
- No HP values are serialized on the combatant element — the character sheet
  is the single source of truth.

#### Monsters (`BattleDialogModelMonsterCombatant` / `BattleDialogModelMonsterBase`)

- `BattleDialogModelMonsterCombatant` already has per-instance HP tracking
  (`hp` field). A **maximum HP** value must also be stored per combatant.
- At combatant creation the maximum HP is set from the monster's HP roll (or
  average) — it is **not** a live reference into `MonsterClassv2`.
- Existing `hp` field serves as current HP.
- Maximum HP is serialized.
- `MonsterClassv2` is **not** modified.

### 4. RuleHealth Integration

- The health bar percentage is computed via the campaign's active `RuleHealth`
  instance rather than a raw ratio, so that rule-specific HP semantics
  (e.g. DaggerHeart wounds) are respected.
- The `RuleHealth` interface must expose (or already expose) a method that
  returns a normalized `[0.0, 1.0]` health fraction given a combatant.
  If that method does not exist, it must be added to `RuleHealth` and
  implemented in all concrete subclasses (`RuleHealth5e`, `RuleHealthDaggerHeart`).

---

## Persistence Requirements

- Campaign setting: serialized as an attribute on the `<Campaign>` XML element
  (or appropriate options sub-element).
- Character combatant HP: **not serialized** on the combatant — read live from
  the linked `Characterv2` object.
- Monster combatant max HP: serialized as an attribute on the monster combatant
  XML element.
- Ruleset HP field names: serialized with the ruleset configuration.

---

## Out of Scope

- Animated transitions between HP values.
- Configurable bar colors or thickness per campaign.
- Displaying numeric HP values on the bar.
- Changes to `MonsterClassv2` data.
- Changes to the character sheet (`Characterv2`) HP fields.

---

## Success / Done Conditions

1. Campaign settings UI has a "Show token health bars" toggle (Qt Designer step).
2. When enabled, every combatant token on **both** the player screen and the DM
   view shows the health bar.
3. When disabled, no health bar is visible on either view.
4. A character combatant at 50% HP shows a half-green bar; editing HP on the
   character sheet updates the bar in real time, and vice versa.
5. A monster combatant whose HP was reduced from max shows a proportionally
   reduced green fill.
6. Saving and reloading the campaign preserves: the setting and monster max HP.
   Character HP is restored correctly from the character sheet on reload.
7. Changing the HP field names in the ruleset causes the health bar to read the
   correct fields.
8. All existing builds pass with no regressions.
