# Deep Dive: Grouped combatant rows vanish from the initiative tracker and crash on map switch

date: 2026-09-23
investigator_model: Claude Opus 4.8
focus: BattleFrame initiative-tracker widget lifetime across combatant removal and map/object switching, with monster groups present.
verdict: Block

## Summary

The disappearing group rows, the map-switch crash, and the "reappear then
vanish again" cycle are all explained by a single root defect:
`BattleFrame::removeSingleCombatant()` deletes a tracker widget by indexing
the **group-compressed `_combatantLayout`** with a **flat model-list index**.
When any monster group is present the two index spaces do not line up, so the
call deletes the *wrong* top-level row — frequently a `CombatantGroupWidget`
or an innocent combatant row — while leaving dangling raw pointers in
`_combatantWidgets` / `_groupWidgets` and a zombie member widget that still
points at the just-deleted combatant. The normal-combatant removal path never
rebuilds the widget list, so the corruption is permanent until the next full
refresh (`recreateCombatantWidgets()` via `setModel()` on reactivation). The
next map/object switch tears the frame down (`clearBattleFrame()` /
`clearCombatantWidgets()`), dereferences the dangling pointers, and crashes.
The model data survives (combatants and groups are owned by the layers/model,
not the widgets), which is why the rows reappear after reactivation and then
disappear again on the next removal. **This is a use-after-free crash path and
a data-visibility corruption; it must be fixed before this code ships.**

## Findings

### Finding 1 — `removeSingleCombatant` indexes the compressed layout with a flat model index — Block

file: [battleframe.cpp](../../battleframe.cpp#L5404-L5417)
category: ownership / undefined-behaviour (use-after-free)

The tail of `removeSingleCombatant()` (non-event path):

```cpp
// Find the index of the removed item
int index = _model->getCombatantList().indexOf(combatant);   // FLAT model index

// Delete the widget for the combatant
_combatantWidgets.remove(combatant);
QLayoutItem *child = _combatantLayout->takeAt(index);        // COMPRESSED layout index
if(child != nullptr)
{
    child->widget()->deleteLater();                          // deletes the WRONG widget
    delete child;
}

_model->removeCombatant(combatant);
```

`_model->getCombatantList()` contains **every** combatant, including each group
member individually. `_combatantLayout` holds only **top-level rows**: ungrouped
combatant widgets plus **one `CombatantGroupWidget` per group** — the group's
members live in the group widget's `contentLayout`, not in `_combatantLayout`.
Therefore `_combatantLayout->count() < _model->getCombatantCount()` whenever a
group exists, and the two index spaces diverge after the first group.

This exact mismatch is already documented and handled correctly elsewhere:

- `getWidgetFromCombatant()` was rewritten to look up by pointer and carries a
  long comment explaining that "`_combatantLayout` only holds the top-level
  rows … while the combatant list contains every group member as a separate
  entry." See [battleframe.cpp](../../battleframe.cpp#L5291-L5312).
- The initiative-event branch **just above** the buggy code removes by
  `_combatantLayout->indexOf(widget)` — the correct approach. See
  [battleframe.cpp](../../battleframe.cpp#L5387-L5399).
- The drag/drop reorder path also uses `_combatantLayout->indexOf(widget)`.
  See [battleframe.cpp](../../battleframe.cpp#L2150-L2160).

Only `removeSingleCombatant`'s normal-combatant path was left indexing the
layout by the model position. Consequences of a single grouped/near-group
removal:

1. **Wrong deletion.** `takeAt(index)` removes and `deleteLater()`s a different
   top-level widget. If `index` maps to the group widget's slot, the **entire
   group row and all its member widgets disappear** (Qt deletes the group
   widget's children). If it maps to another combatant's row, **that unrelated
   combatant vanishes** from the tracker.
2. **Dangling map entries.** `_combatantWidgets` (for the wrongly-deleted
   ungrouped combatant) and/or `_groupWidgets` (for the wrongly-deleted group)
   retain **raw pointers to freed widgets**. Both maps are raw-pointer maps
   (see Finding 3), so `if(ptr)` guards do not catch them.
3. **Zombie member widget.** The combatant actually being removed is a group
   member; its widget lives inside the group, never in `_combatantLayout`, so
   `takeAt(index)` never touches it. After `_model->removeCombatant(combatant)`
   `delete`s the combatant, that member widget is still alive and still returns
   the freed combatant from `getCombatant()` (`_combatant` is a stored raw
   pointer, [combatanttemplateframe.cpp](../../combatanttemplateframe.cpp#L100-L102)).
   Any subsequent paint, `handleGroupClicked`, or `updateMasterCheckboxes` on
   that row is a use-after-free.
4. **Out-of-range case.** When `index >= _combatantLayout->count()`,
   `takeAt(index)` returns `nullptr`: nothing is removed from the layout, but
   the entry is still dropped from `_combatantWidgets` and the combatant is
   deleted — leaving a visible ghost row bound to a freed combatant.

**Recommended action:** Remove by widget pointer, not model index: look up
`CombatantWidget* w = _combatantWidgets.value(combatant)`, detach it from its
group (`_groupWidgets.value(groupId)->removeMemberWidget(w)`) when grouped else
`_combatantLayout->takeAt(_combatantLayout->indexOf(w))`, then `w->deleteLater()`;
follow the removal with `recreateCombatantWidgets()` (as the event path does).

---

### Finding 2 — Normal combatant removal never rebuilds the tracker — Block (makes Finding 1 permanent)

file: [battleframe.cpp](../../battleframe.cpp#L2949-L2960)
category: correctness

`handleCombatantRemove()` only rebuilds the widget list when an **initiative
event** was removed:

```cpp
bool removedInitiativeEvent = false;
for(BattleDialogModelCombatant* selectedCombatant : std::as_const(combatantsToRemove))
{
    if(selectedCombatant && (selectedCombatant->getCombatantType() == DMHelper::CombatantType_InitiativeEvent))
        removedInitiativeEvent = true;
    removeSingleCombatant(selectedCombatant, !isBatchRemoval);
}

if(removedInitiativeEvent)
    recreateCombatantWidgets();
```

For monster/character removals there is no rebuild, so the in-place (and, per
Finding 1, incorrect) layout mutation is the final state. Every other mutation
that changes group membership — the drag-drop group add/remove and the
initiative-event removal — already calls `recreateCombatantWidgets()` or
`reorderCombatantWidgets()`. The normal removal path is the outlier that leaves
the tracker in a corrupted state.

**Recommended action:** Call `recreateCombatantWidgets()` after any combatant
removal (not just initiative events); it deterministically rebuilds `_groupWidgets`,
re-tints the active row, and prunes stale rows from the intact model.

---

### Finding 3 — `_combatantWidgets` / `_groupWidgets` are raw-pointer maps; teardown dereferences dangling entries on map switch — Block (crash surface)

file: [battleframe.h](../../battleframe.h#L442-L443), [battleframe.cpp](../../battleframe.cpp#L5504-L5516), [battleframe.cpp](../../battleframe.cpp#L4882-L4906)
category: ownership / lifetime

```cpp
QMap<BattleDialogModelCombatant*, CombatantWidget*> _combatantWidgets;
QMap<QUuid, CombatantGroupWidget*> _groupWidgets;
```

Neither uses `QPointer`, so once Finding 1 has `deleteLater()`d a widget the map
still holds a pointer to freed memory, and the `if(widget)` / `if(groupWidget)`
guards throughout the frame pass on garbage. The crash fires on the next
object/map switch. `MainWindow::handleTreeItemSelected()` →
`MainWindow::deactivateObject()` → `BattleFrame::deactivateObject()` →
`setBattle(nullptr)` → `setModel(nullptr)`
([mainwindow.cpp](../../mainwindow.cpp#L2509-L2524),
[battleframe.cpp](../../battleframe.cpp#L4352-L4354)) runs:

- `clearBattleFrame()` — iterates `_combatantWidgets` and calls
  `widget->disconnectInternals()` on each, dereferencing a freed
  `CombatantTemplateFrame`. [battleframe.cpp](../../battleframe.cpp#L5504-L5515)
- `clearCombatantWidgets()` — iterates `_groupWidgets`, calls
  `groupWidget->getMemberWidgets()` on a freed group widget, then
  `qDeleteAll(_groupWidgets)` double-frees it.
  [battleframe.cpp](../../battleframe.cpp#L4882-L4900)

Either dereference is the reported crash on "changed map."

Note this is distinct from the internal `CombatantGroupWidget::_memberWidgets`
list, which *is* correctly `QList<QPointer<CombatantWidget>>` and prunes nulls
in `updateMasterCheckboxes()`. The hardening that was applied to the group's
member list was never applied to the two frame-level maps.

**Recommended action:** Fixing Finding 1 removes the dangling entries at the
source. As defense-in-depth, make `_groupWidgets`/`_combatantWidgets` hold
`QPointer` (or null-and-skip on delete) so a stray deletion can never turn the
teardown paths into a use-after-free.

---

### Finding 4 — Removed group member is never detached from its group widget — Warn

file: [battleframe.cpp](../../battleframe.cpp#L5404-L5417), [combatantgroupwidget.cpp](../../combatantgroupwidget.cpp#L63-L88)
category: ownership

Because a member widget is parented into `CombatantGroupWidget::contentLayout`
(via `addMemberWidget()`), `removeSingleCombatant` — which only ever touches
`_combatantLayout` — cannot remove it and never calls
`groupWidget->removeMemberWidget(widget)`. After the combatant is deleted the
member widget remains in the group with a dangling `_combatant`. `getMemberWidgets()`
returns it (the `QPointer` is still valid — the *widget* wasn't deleted, only
the combatant), so group aggregation and `handleGroupClicked()` → `getCombatant()`
touch freed combatant memory.

**Recommended action:** Detach the member from its group widget before deleting
it (covered by the Finding 1 fix); switching the normal-removal path to a full
`recreateCombatantWidgets()` also resolves this.

---

### Finding 5 — Batch removal compounds the index drift — Warn

file: [battleframe.cpp](../../battleframe.cpp#L2949-L2956)
category: correctness

The batch branch calls `removeSingleCombatant(selectedCombatant, false)` in a
loop. Each iteration removes one entry from `_model->_combatants` (shifting all
later flat indices) while the layout is separately mutated, so the second and
subsequent removals in a multi-select delete are even more misaligned than a
single removal. Same root cause as Finding 1; called out because a multi-token
delete is a plausible live-session action and will delete several wrong rows at
once.

**Recommended action:** Resolved by the Finding 1/2 fixes (remove-by-pointer +
single rebuild after the batch).

---

### Finding 6 — `removeSingleCombatant` bypasses group bookkeeping; empty groups are not pruned — Info

file: [battleframe.cpp](../../battleframe.cpp#L5417), [battledialogmodel.cpp](../../battledialogmodel.cpp#L389-L407), [battledialogmodel.cpp](../../battledialogmodel.cpp#L610-L633)
category: correctness

`_model->removeCombatant()` deletes the combatant from its token layer but does
not call `removeCombatantFromGroup()`, so removing the last member of a group
leaves an empty `BattleDialogModelCombatantGroup` in `_groups` (and thus an
empty group row after a rebuild). Not a crash, but it desynchronizes the model's
group set from what the tracker should show and compounds the confusion in
Findings 1–2.

**Recommended action:** In the removal path, call
`_model->removeCombatantFromGroup(combatant)` (which already prunes now-empty
groups) before `_model->removeCombatant(combatant)`.

---

### Finding 7 — Recursive event-filter double-release is currently mitigated — Info

file: [battleframe.cpp](../../battleframe.cpp#L2038-L2062), [combatantwidget.cpp](../../combatantwidget.cpp#L66-L88)
category: correctness

`installEventFilterRecursive()` installs the `BattleFrame` filter on each row
widget plus its children/grandchildren, so one physical click can surface
multiple `MouseButtonRelease` callbacks. The handler guards against this with
the shared `_mouseDown` flag and clears it on the first release, so the
row-click/selection logic runs once. This matches the prior-investigation note
and is presently benign; flagged only because the guard is a single shared
`bool` on `BattleFrame` and would break if row interaction ever became
re-entrant. Not related to the crash.

**Recommended action:** None required now; keep in mind if the selection/rollover
logic is reworked.

## Verdict Rationale

Findings 1–3 form a single crash path: an incorrect index (Finding 1) creates
dangling widget pointers, the missing rebuild (Finding 2) makes them permanent,
and the raw-pointer teardown (Finding 3) dereferences them on the next map/object
switch. That is a use-after-free reachable by ordinary live-session actions
(remove a grouped monster, then change map), so the overall verdict is **Block**.
The model layer is not corrupted — combatants and groups are owned by the
layers/model and survive — which is exactly why the rows return after
reactivation and vanish again on the next removal.

## Root-cause hypotheses per reported symptom

1. **Grouped monster rows disappear from the tracker.** Finding 1: a removal
   whose flat model index coincides with the `CombatantGroupWidget`'s slot in
   `_combatantLayout` deletes the group widget (and its child member widgets)
   instead of the intended row. If the index maps to another top-level row,
   that unrelated combatant disappears instead. No `recreateCombatantWidgets()`
   runs afterward (Finding 2), so the loss sticks.

2. **Crash on changing map.** Finding 3: the dangling `_combatantWidgets` /
   `_groupWidgets` entries left by Finding 1 are dereferenced when the map/object
   switch tears the frame down via `setModel(nullptr)` →
   `clearBattleFrame()` (`disconnectInternals()` on a freed widget) and
   `clearCombatantWidgets()` (`getMemberWidgets()` + `qDeleteAll` on a freed
   group widget).

3. **Rows return after activating another entry and coming back, then vanish
   again.** Reactivation runs `setModel(model)` →
   `recreateCombatantWidgets()` → `clearCombatantWidgets()` + full
   `buildCombatantWidgets()` from the intact model, rebuilding every group and
   member from scratch ([battleframe.cpp](../../battleframe.cpp#L532-L539),
   [battleframe.cpp](../../battleframe.cpp#L4908-L5008)). The next grouped
   removal re-triggers Finding 1 and the rows disappear again.

## Recommended reproduction sequence

1. Open/create a 5e battle with a token layer; add two PCs and three monsters.
2. Select the three monsters and Group them; roll/sort so the group lands in a
   middle initiative slot. The tracker now shows three top-level rows
   (`PC`, `GroupWidget`, `PC`) while the model list has five entries.
3. Right-click the **first** group member and choose **Remove** (its flat model
   index collides with the group widget's layout slot). Observe an unrelated row
   — or the whole group — vanish from the tracker rather than just that monster.
4. Switch the campaign-tree selection to a Map (or any other object). Expect a
   crash in `clearBattleFrame()` / `clearCombatantWidgets()`.
5. If it does not crash immediately, reactivate the battle: the group reappears
   (rebuilt from the model). Remove another grouped monster and repeat — it
   disappears again.

A multi-select delete of grouped tokens (Finding 5) reproduces the disappearance
even more readily.

## Next Steps

- Fix Finding 1 by removing the widget by pointer (detach from group when
  grouped, else `takeAt(indexOf(widget))`), never by `_model` list index.
- Fix Finding 2 by calling `recreateCombatantWidgets()` after every combatant
  removal, matching the initiative-event and drag-drop membership paths.
- Fix Finding 6 by routing removal through `removeCombatantFromGroup()` so empty
  groups are pruned and `_groupWidgets` stays consistent.
- Harden Finding 3 by making `_combatantWidgets` / `_groupWidgets` `QPointer`
  maps (or null-and-skip on delete) so teardown can never UAF.
- Re-verify with the reproduction sequence above and with a batch (multi-select)
  removal while a group is present.

--- HANDOFF TO COORDINATOR — DEEP DIVE BLOCK ---

Deep Dive Agent returned Block.

Investigation slug: grouped-combatant-tracker-map-switch-crash
Investigation file: DMHelper/src/dev/investigations/grouped-combatant-tracker-map-switch-crash.md
Block finding(s):
  - removeSingleCombatant indexes the group-compressed _combatantLayout with a flat model index: deletes the wrong tracker widget and leaves dangling _combatantWidgets/_groupWidgets pointers.
  - Normal combatant removal never rebuilds the tracker, making the corruption permanent until reactivation.
  - Raw-pointer widget maps are dereferenced on map/object switch teardown (use-after-free crash).

Recommended action: fix-in-place patch (remove-by-pointer + rebuild after removal + QPointer hardening). No new plan/design document required.

If a new plan is needed, dispatch the Design Agent with this spec summary as the
starting point and reference the investigation file as supporting context.
---
