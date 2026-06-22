# Spellbook Effect Panel Integration

## Summary
`SpellbookTemplateDialog` was built as the new template-driven replacement for `SpellbookDialog`. The new dialog already has the effect-panel UI widgets in `spellbooktemplatedialog.ui`, but none of the backing code to populate, read, or interact with those widgets was carried over. The old `SpellbookDialog` (preserved as `_OLD` files) contains the complete reference implementation. This feature ports all effect-panel behavior into `SpellbookTemplateDialog`, adapted for the `Spellv2` API.

## User-visible behavior
- The Effect section in the Spellbook dialog works: selecting an effect type enables/disables size fields and the shape group; symmetric types (Radius, Cone, Cube) keep width and height in sync
- The shape preview image (`lblEffectImage`) updates in real-time as color, opacity, type, and token are changed
- The color picker and opacity slider control the rendered shape
- Token browse button opens a file picker; rotate CW/CCW buttons rotate it; the "2-Minute Tabletop" credit labels appear/hide based on the file path
- "Edit..." conditions button opens `ConditionsEditDialog` and round-trips to `Spellv2::getEffectConditionList()` / `setEffectConditionList()`
- Condition icons are shown in `frameConditions` using `Conditions::activeConditions()->getConditionIconPath()`
- All effect data is persisted to the `Spellv2` object whenever a control changes (no separate Save button needed)
- On `setSpell()`, all effect controls are populated from the `Spellv2` object

## Subsystems
- UI shell
- Spellbook

## Done conditions
- Opening the Spellbook dialog and selecting a spell shows the effect type, size, color, opacity, token path, and conditions populated from the spell
- Changing any effect control and navigating to another spell then back shows the change persisted
- The shape preview renders the correct shape / token image

## Open questions
None.
