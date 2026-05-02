# Spec: Token Health Bar

**Feature slug:** `token-health-bar`  
**Subsystem:** Battle / Tokens (GL player screen)  
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

### 2. Rendering (Player GL Screen)

- When the setting is `true`, each combatant token drawn in
  `PublishGLBattleToken` (or equivalent token-rendering class) shows a
  rectangular health bar overlay above the token image.
- Bar dimensions: proportional to the token's rendered size (e.g. full token
  width, ~8% of token height).
- Bar position: immediately above the token image, centered horizontally.
- Visual: red background, green foreground scaled by `currentHP / maxHP`.
  When `currentHP <= 0`, bar is entirely red.
- The bar is drawn using the existing OpenGL infrastructure (quads / shader
  primitives already used for token highlights and overlays).

### 3. Health Data per Combatant

#### Characters (PCs and NPCs — `BattleDialogModelCharacter`)

- `BattleDialogModelCharacter` stores an explicit **current HP** value and an
  explicit **maximum HP** value per combatant entry.
- Defaults at combatant creation: read from the linked `Characterv2` fields
  (`hit_points` → current; `maximumHp` → max).
- These per-combatant values are independent of the character sheet after
  creation — they represent the in-battle state.
- Both values are serialized.

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
- Character combatant HP: serialized as attributes on the combatant XML element.
- Monster combatant max HP: serialized as an attribute on the monster combatant
  XML element.

---

## Out of Scope

- Health bar on the **DM view** (scene graphics items) — DM view only.
- Animated transitions between HP values.
- Configurable bar colors or thickness per campaign.
- Displaying numeric HP values on the bar.
- Changes to `MonsterClassv2` data.
- Changes to the character sheet (`Characterv2`) HP fields.

---

## Success / Done Conditions

1. Campaign settings UI has a "Show token health bars" toggle (Qt Designer step).
2. When enabled, every combatant token on the player screen shows the health bar.
3. When disabled, no health bar is visible.
4. A character combatant at 50% HP shows a half-green bar.
5. A monster combatant whose HP was reduced from max shows a proportionally
   reduced green fill.
6. Saving and reloading the campaign preserves: the setting, character HP
   values, and monster max HP.
7. All existing builds pass with no regressions.
