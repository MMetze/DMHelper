# Deep Dive: Combatant + combatant-widget subsystem architecture review

date: 2026-09-23
investigator_model: Claude Opus 4.8
focus: Forward-looking architecture/design review of the combatant model layer, combatant-widget layer, and BattleFrame's model/widget/layout bookkeeping — independent of the already-fixed `removeSingleCombatant` crash.
verdict: Warn

## Summary

The combatant subsystem is a three-layer design — model list, a
`combatant -> widget` map, and a group-compressed `QVBoxLayout` — that must be
kept in sync **by hand** at ~17 call sites. The model is clean and its
ownership story is sound (combatants owned by token layers; the model list is
an initiative-ordering view). The fragility lives entirely in `BattleFrame`,
where two incompatible index spaces (the **flat** model list and the
**compressed** layout) coexist. That mismatch already produced one Block-level
crash (now fixed) and still produces a second, latent correctness bug in
`getNextCombatant()`. The widget maps remain raw pointers, and widgets hold raw
back-pointers to model combatants, so the crash surface that the prior fix
closed at one call site is still open in principle. Nothing here is a new
crash or data-loss path on the current happy path, so the overall verdict is
**Warn**: real design-quality issues and one functional bug that should be
fixed, plus a clear opportunity to collapse three hand-synced structures into
one coordinator.

## Scope & method

This review was requested as a read-only, forward-looking design review (not a
bug repro). No code was changed. In lieu of a blocking clarify round, the
scope was taken as the seven areas enumerated in the request:

1. Model layer (`BattleDialogModelCombatant` + subclasses, `…Group`,
   `…InitiativeEvent`, `BattleDialogModel` storage).
2. Widget layer (`CombatantWidget`/`…Base`, `CombatantTemplateFrame`,
   `CombatantGroupWidget`, `InitiativeEventWidget`).
3. Controller layer (`BattleFrame` maps, `_combatantLayout`, sync points).
4. Lifetime/ownership across the model↔widget boundary.
5. Runtime performance (linear scans, full rebuilds).
6. Legibility (duplicated branches, dead code).
7. Concrete simplifications ranked by effort vs. impact.

Findings are framed as **design-quality** severities (Block / Warn / Info),
not as an assertion that each is an active crash.

---

## Current architecture

### Layers and ownership

```
Combatant (statblock, owned elsewhere)
   ^  raw Combatant* (BattleDialogModelCombatant::_combatant)
   |
BattleDialogModelCombatant  (QObject; OWNED BY LayerTokens, not the model)
   |   subclasses: BattleDialogModelCharacter
   |               BattleDialogModelMonsterCombatant : …MonsterBase
   |               BattleDialogModelInitiativeEvent  (synthetic, no token)
   |
BattleDialogModel  (initiative-ordering VIEW + group/effect/event registry)
   |   _combatants  : QList<…Combatant*>          (flat, every member listed)
   |   _groups      : QList<…CombatantGroup*>      (metadata only)
   |   _initiativeEvents : QList<…InitiativeEvent*> (ALSO present in _combatants)
   |   _effects     : QList<…Effect*>
   |
BattleFrame  (controller / tracker UI)
   |   _combatantWidgets : QMap<…Combatant*, CombatantWidget*>   (raw ptrs)
   |   _groupWidgets     : QMap<QUuid, CombatantGroupWidget*>    (raw ptrs)
   |   _combatantLayout  : QVBoxLayout  (COMPRESSED: top-level rows only)
   |
   +-- CombatantWidget (abstract row)
         +-- CombatantTemplateFrame  (monster/character; raw _combatant back-ptr)
         +-- InitiativeEventWidget   (raw _event back-ptr)
   +-- CombatantGroupWidget  (owns QList<QPointer<CombatantWidget>> _memberWidgets)
```

Ownership summary:

- **Combatants** are owned by `LayerTokens`. `BattleDialogModel::_combatants`
  is documented as "for initiative sorting only"
  ([battledialogmodel.h](../../battledialogmodel.h#L162-L167)).
  `removeCombatant()` removes from the token layer and `delete`s
  ([battledialogmodel.cpp](../../battledialogmodel.cpp#L389-L407)).
- **Groups / initiative events / effects** are owned by the model and
  `qDeleteAll`'d in its destructor
  ([battledialogmodel.cpp](../../battledialogmodel.cpp#L44-L50)).
- **Combatant widgets** are owned by `BattleFrame` via the `_combatantWidgets`
  map and are parented to `ui->scrollAreaWidgetContents`. They are cached and
  reused across rebuilds — `clearCombatantWidgets()` does **not** delete them,
  only the group widgets ([battleframe.cpp](../../battleframe.cpp#L4872-L4906)).
  They are deleted only in `removeSingleCombatant`, `clearBattleFrame`, and the
  destructor.
- **Group widgets** are recreated from scratch on every `buildCombatantWidgets`
  and `qDeleteAll`'d on every `clearCombatantWidgets`.
- **`CombatantGroupWidget::_memberWidgets`** correctly uses
  `QList<QPointer<CombatantWidget>>` and prunes nulls
  ([combatantgroupwidget.h](../../combatantgroupwidget.h#L66)) — this hardening
  was **not** propagated to the two `BattleFrame` maps.

### The two index spaces (the central hazard)

- **Flat model space**: `_model->getCombatantList()` / `getCombatant(i)` —
  every combatant, including each group member individually.
- **Compressed layout space**: `_combatantLayout` — top-level rows only:
  ungrouped combatant widgets **plus one `CombatantGroupWidget` per group**.
  Group members live inside `CombatantGroupWidget::contentLayout`, not in
  `_combatantLayout`. The layout has no trailing stretch (it uses
  `AlignTop`), so `_combatantLayout->count()` is exactly the top-level row
  count ([battleframe.cpp](../../battleframe.cpp#L213-L216)).

Whenever a group exists, `_combatantLayout->count() < _model->getCombatantCount()`.
Any code that mixes the two spaces is a latent defect. The correct patterns in
the codebase always translate via the widget pointer
(`_combatantWidgets.value(combatant)` then `_combatantLayout->indexOf(widget)`).

### Model → widget synchronisation points

Sync is **manual and convention-based**, not signal-driven for structure:

```
model mutation ──(by convention)──> BattleFrame::recreateCombatantWidgets()
                                        clearCombatantWidgets()   (drop group widgets, keep combatant widgets)
                                        buildCombatantWidgets()   (recreate group widgets, lay out in model order)
                                        reorderCombatantWidgets() (tear down + lay out in model order AGAIN)
```

`recreateCombatantWidgets()` is invoked from ~17 call sites
([battleframe.cpp](../../battleframe.cpp#L532-L540) and callers at lines 1344,
1383, 1462, 1697, 2206, 2240, 2742, 2959, 3714, 3723, 3753, 3766, 3775, 4024,
4080, 4411, 4512). The model *does* emit structural signals
(`combatantAdded/Removed`, `groupAdded/Removed/Changed`, `initiativeOrderChanged`)
but `BattleFrame` connects only `combatantAdded`→`handleCombatantAdded` (which
just wires per-combatant signals, it does **not** build a widget) and
`combatantRemoved`→`handleCombatantRemoved`
([battleframe.cpp](../../battleframe.cpp#L4399-L4402)). The group signals are
emitted and consumed by nobody (see F6).

Per-combatant *content* updates (HP, conditions, visibility) are correctly
scoped: they call `updateCombatantWidget()` /`updateCombatantIcon()` and do
**not** trigger a full rebuild ([battleframe.cpp](../../battleframe.cpp#L3896-L3906)).
Only structural changes (add/remove/group/ungroup/drag-between-groups) trigger
`recreateCombatantWidgets()`. That granularity is correct — the cost problem is
in what a single rebuild does (F5), not in over-triggering on HP changes.

---

## Findings

### F1 — Three parallel hand-synced structures across two index spaces — Warn (systemic)

file: [battleframe.h](../../battleframe.h#L437-L446), [battleframe.cpp](../../battleframe.cpp#L4908-L5087)
category: correctness / maintainability

`_combatants` (model, flat), `_combatantWidgets` (map), and
`_combatantLayout`+`_groupWidgets` (compressed) are three representations of the
same ordered set, kept consistent by discipline at every mutation site. The
flat↔compressed translation is implicit and re-derived ad hoc in each function.
This is the root cause of the already-fixed Block crash and of F2 below, and it
is why there are ~17 explicit `recreateCombatantWidgets()` calls: the safest
way the code found to stay correct is to throw the whole tracker away and
rebuild it. Every new feature that mutates membership must remember to do so.

**Recommended action:** Introduce a single coordinator (see Recommendation R8)
that owns the mapping and the flat↔compressed translation, so no caller
manipulates `_combatantLayout` indices directly.

---

### F2 — `getNextCombatant` guards flat iteration with the compressed row count — Warn (real bug)

file: [battleframe.cpp](../../battleframe.cpp#L5326-L5366)
category: correctness

```cpp
int nextCombatantIndex = _model->getCombatantList().indexOf(combatant); // FLAT
if(_combatantLayout->count() <= 1)                                      // COMPRESSED
    return nullptr;
```

The early-out uses the top-level row count but the loop advances through the
flat model list. With a single group that contains **all** combatants and no
ungrouped rows, `_combatantLayout->count() == 1`, so the guard returns
`nullptr` even though there are several combatants to cycle. Effect: "Next"
fails to advance, and the active-combatant reassignment in
`removeSingleCombatant` (which calls `next()` when `getCombatantCount() > 1`)
mis-fires in the all-grouped case. This is the same flat-vs-compressed class as
the fixed crash, surviving here as a functional (non-crashing) bug.

**Recommended action:** Guard on navigable combatant count, e.g.
`if(_model->getCombatantCount() <= 1) return nullptr;` (the loop already skips
dead/unknown/hidden), and drop the dependency on `_combatantLayout->count()`.

---

### F3 — `_combatantWidgets` / `_groupWidgets` are raw-pointer maps — Warn (latent crash surface)

file: [battleframe.h](../../battleframe.h#L445-L446)
category: ownership / lifetime

```cpp
QMap<BattleDialogModelCombatant*, CombatantWidget*> _combatantWidgets;
QMap<QUuid, CombatantGroupWidget*> _groupWidgets;
```

Neither uses `QPointer`, so any stray `deleteLater()`/`delete` of a widget
leaves a dangling map entry that the pervasive `if(widget)` / `if(groupWidget)`
guards cannot catch. The teardown paths dereference these maps
(`clearBattleFrame` calls `disconnectInternals()` on every value
[battleframe.cpp](../../battleframe.cpp#L5514-L5537); `clearCombatantWidgets`
calls `getMemberWidgets()` then `qDeleteAll` on every group widget
[battleframe.cpp](../../battleframe.cpp#L4872-L4906)). This is exactly the
mechanism that turned the fixed indexing bug into a use-after-free on map
switch. The fix removed the *source* of dangling entries at one site; the
*surface* is still unhardened. Note the contrast: the group's internal member
list already uses `QPointer` and prunes nulls — the same discipline is simply
missing one level up.

**Recommended action:** Make both maps hold `QPointer` (or centralise deletion
through a helper that nulls-and-removes), so a stray deletion can never turn
teardown into a UAF.

---

### F4 — Widgets hold raw back-pointers to model combatants — Warn (zombie risk)

file: [combatanttemplateframe.h](../../combatanttemplateframe.h#L84), [combatanttemplateframe.cpp](../../combatanttemplateframe.cpp#L98-L102), [initiativeeventwidget.h](../../initiativeeventwidget.h#L44)
category: ownership / lifetime

`CombatantTemplateFrame::_combatant` and `InitiativeEventWidget::_event` are
raw pointers, and `getCombatant()` returns them directly. If a widget ever
outlives its combatant (the prior investigation's "zombie member widget"),
`getCombatant()` yields a dangling pointer that flows into
`handleGroupClicked` → `setSelectedCombatant`, paint, and aggregation. The
checkbox→model connections made in the constructor
([combatanttemplateframe.cpp](../../combatanttemplateframe.cpp#L71-L88)) are
auto-severed by Qt on destruction of either party, so those are safe; the raw
data pointer is the exposure. Removal is now ordered widget-first, so this is
latent rather than live, but it is defense worth adding given the frame relies
on it.

**Recommended action:** Store the back-pointer as `QPointer` (the model
combatant is a `QObject`) and null-check in `getCombatant()`, or have the
owning frame proactively call a `clearCombatant()` before the model object is
destroyed.

---

### F5 — `recreateCombatantWidgets` recreates all group widgets and lays out twice — Warn (performance)

file: [battleframe.cpp](../../battleframe.cpp#L532-L540), [battleframe.cpp](../../battleframe.cpp#L4908-L5087)
category: performance

A single structural change runs `clearCombatantWidgets` +
`buildCombatantWidgets` + `reorderCombatantWidgets`. Two concrete inefficiencies:

1. **Group widgets churn every time.** `clearCombatantWidgets` `qDeleteAll`s all
   group widgets and `buildCombatantWidgets` `new`s them all back — even when a
   single ungrouped combatant was added. Combatant widgets are cached (good),
   but group widgets are not.
2. **The layout is built twice.** `buildCombatantWidgets` already adds every row
   to `_combatantLayout` in model order (adding each group at its first
   member), then `reorderCombatantWidgets` immediately `takeAt`s everything,
   removes every member from every group, and re-adds in the same model order.
   For a fresh rebuild the second pass reproduces the first. Each
   `addMemberWidget`/`removeMemberWidget` also calls `updateMasterCheckboxes()`,
   so members are re-aggregated multiple times per rebuild.

With a large roster (dozens of monsters across several groups) each add/remove
does O(n) widget re-parenting twice plus O(groups) widget alloc/free, which is
visible UI churn. Not catastrophic (no per-HP or per-frame rebuild), but it
scales poorly and is mostly redundant work.

**Recommended action:** Short term — drop the redundant `reorderCombatantWidgets`
from `recreateCombatantWidgets` (verify group ordering parity first). Medium
term — reuse group widgets across rebuilds (keyed by group id) instead of
delete/recreate, and move to incremental add/remove/move (R9).

---

### F6 — Structural model signals are emitted but never consumed — Info

file: [battledialogmodel.cpp](../../battledialogmodel.cpp#L563-L629), [battledialogmodelcombatantgroup.cpp](../../battledialogmodelcombatantgroup.cpp#L41-L71)
category: maintainability

`BattleDialogModel::groupAdded/groupRemoved/groupChanged` and
`BattleDialogModelCombatantGroup::groupChanged` are emitted on every group
mutation but are connected to no slot anywhere (only `initiativeOrderChanged`
is consumed — by `PublishGLBattleRenderer`, not the tracker
[publishglbattlerenderer.cpp](../../publishglbattlerenderer.cpp#L230)). The
tracker instead re-syncs via manual `recreateCombatantWidgets()` calls next to
each mutation. This is dead API surface and a missed opportunity: the signals
already exist to drive the very sync that is currently open-coded, and their
presence implies a wiring that does not exist (a trap for the next author).
Note also: editing a group's initiative in `CombatantGroupWidget` mutates the
group and emits `groupChanged`, but nothing re-sorts the tracker until a manual
Sort — a user-visible consequence of the unconsumed signal.

**Recommended action:** Either delete the unused signals, or (preferably, as
part of R8) connect them to a coordinator so structure changes are event-driven
and the ~17 manual calls collapse.

---

### F7 — Duplicate unreachable block in `getWidgetFromCombatant` — Info (confirmed dead code)

file: [battleframe.cpp](../../battleframe.cpp#L5291-L5324)
category: maintainability

Verified as reported. The function body contains the explanatory comment and
`return _combatantWidgets.value(combatant, nullptr);` **twice**; lines
~5313-5324 (the second comment block and second `return`) are unreachable after
the first `return` at ~5312. Harmless but confusing — likely a paste artifact
from the earlier fix.

**Recommended action:** Delete the second comment+return block.

---

### F8 — O(n) reverse map lookups on the selection/drag hot path — Info (performance + legibility)

file: [battleframe.cpp](../../battleframe.cpp#L2009), [battleframe.cpp](../../battleframe.cpp#L2050)
category: performance

Mouse-move (drag start) and mouse-release (selection) resolve the combatant via
`_combatantWidgets.key(widget, nullptr)`, a linear scan of the whole map on
every click. The widget already exposes `getCombatant()` (O(1), used a few
lines later in the same handler), which returns the identical mapping. The
`.key()` scan is both slower and redundant.

**Recommended action:** Replace `_combatantWidgets.key(widget, nullptr)` with
`widget->getCombatant()`.

---

### F9 — Three divergent code paths for conceptually identical operations — Info (legibility)

file: [battleframe.cpp](../../battleframe.cpp#L5368-L5418)
category: maintainability

`removeSingleCombatant` has near-duplicate branches for the initiative-event
case and the normal case (both: read+remove from `_combatantWidgets`, detach
from layout/group, `deleteLater`). More broadly, ungrouped combatants, grouped
members, and initiative events each get their own handling in build/reorder/
remove/active-highlight, though they are the same abstract operation ("a row in
the tracker"). The divergence is what let one branch drift out of correctness.

**Recommended action:** Extract a single `detachAndDeleteRowWidget(widget)`
helper used by both branches; longer term, treat all three row kinds through one
row abstraction (R8).

---

### F10 — Derived + duplicated "truth" for grouping and initiative events — Info

file: [battledialogmodelcombatant.h](../../battledialogmodelcombatant.h#L44-L45), [battledialogmodel.cpp](../../battledialogmodel.cpp#L525-L633), [battledialogmodel.cpp](../../battledialogmodel.cpp#L438-L468)
category: correctness / maintainability

Two overlapping representations exist:

- **Group membership** is single-sourced on the combatant (`_groupId`); the
  `…CombatantGroup` object holds only metadata (name, initiative, collapsed).
  `getGroupMembers()` derives membership by scanning `_combatants`
  ([battledialogmodel.cpp](../../battledialogmodel.cpp#L525-L540)). This is a
  reasonable single-source design, but empty-group pruning is manual
  (`removeCombatantFromGroup`), and `removeCombatant` does **not** prune — so a
  group can be left empty unless callers route through the right method (the
  fixed `removeSingleCombatant` now does).
- **Initiative events** are stored **twice**: in `_initiativeEvents` and again
  in `_combatants` (via `appendInitiativeEvent` → `appendCombatantToList`
  [battledialogmodel.cpp](../../battledialogmodel.cpp#L438-L455)). Both must be
  kept in lockstep on removal. It works today but is a duplication that any new
  bulk operation must respect.

**Recommended action:** No change required for correctness now; document the
invariants (membership derived from `_groupId`; events mirrored into
`_combatants`) at the member declarations, and consider having
`removeCombatant` always route membership cleanup through
`removeCombatantFromGroup` so the group set can never desync.

---

### F11 — Repeated O(n)/O(g) model scans — Info

file: [battledialogmodel.cpp](../../battledialogmodel.cpp#L336-L345), [battledialogmodel.cpp](../../battledialogmodel.cpp#L508-L540)
category: performance

`getCombatantById` (O(n)), `getGroup` (O(g)), and `getGroupMembers` (O(n)) are
linear and are called inside rebuild loops and sort's `effectiveInitiative`
lambda ([battledialogmodel.cpp](../../battledialogmodel.cpp#L967-L1005), which
calls `getGroup` per comparison → effectively O(n log n · g) per sort). Fine at
today's encounter sizes; worth noting if rosters grow large. Not a correctness
issue.

**Recommended action:** If large-encounter perf becomes a concern, cache a
`QHash<QUuid, group*>` and a per-group member index; otherwise leave as-is.

---

## Verdict rationale

No new crash or data-loss path was found on the current (post-fix) happy path,
so this is not a Block. But F2 is a genuine functional bug, and F3/F4 keep a
use-after-free surface open as a matter of principle (raw widget maps and raw
widget→model back-pointers). Together with the systemic fragility of F1/F5,
these are real design-quality defects that should be addressed — hence **Warn**
rather than Pass. The model layer itself is sound; the risk is concentrated in
`BattleFrame`'s hand-synced bookkeeping.

## Prioritized recommendations (effort vs. impact)

### Small, safe wins (do these first)

- **R1 — Delete the dead block in `getWidgetFromCombatant` (F7).** Trivial;
  removes confusion. No behaviour change.
- **R2 — Replace `_combatantWidgets.key(widget)` with `widget->getCombatant()`
  (F8).** Trivial; O(n)→O(1) on every click; clearer intent.
- **R3 — Fix the `getNextCombatant` guard (F2).** Small; fixes a real
  navigation bug in the all-grouped case. Guard on
  `_model->getCombatantCount()`.
- **R4 — `QPointer` the two `BattleFrame` maps (F3).** Small; closes the latent
  UAF surface with no design change. Mirrors the discipline already used in
  `CombatantGroupWidget::_memberWidgets`.
- **R5 — `QPointer` (or proactively clear) the widget→combatant back-pointers
  (F4).** Small–medium; makes `getCombatant()` null-safe.

### Medium (contained refactors)

- **R6 — Remove the redundant `reorderCombatantWidgets` from
  `recreateCombatantWidgets`, and reuse group widgets across rebuilds (F5).**
  Medium; roughly halves layout churn per structural change. Verify group-row
  ordering parity before removing the second pass.
- **R7 — Extract `detachAndDeleteRowWidget()` and unify the two removal
  branches (F9).** Medium; removes the duplication that caused the original
  drift.

### Larger refactors (higher impact, more risk — discuss before starting)

- **R8 — Introduce a `CombatantTrackerView` coordinator (F1, F6, F9, F10).** A
  single object owns `_combatantWidgets`, `_groupWidgets`, and
  `_combatantLayout`, encapsulates the flat↔compressed translation (no caller
  ever indexes `_combatantLayout` by a model position again), and is driven by
  the model's existing `combatantAdded/Removed`, `groupAdded/Removed/Changed`,
  and `initiativeOrderChanged` signals — collapsing the ~17 manual
  `recreateCombatantWidgets()` calls. High impact on correctness and
  maintainability; invasive — touches most of `BattleFrame`'s tracker code and
  should be planned via the Design Agent, using this document and the prior
  crash investigation as input.
- **R9 — Incremental row updates (add/remove/move one row) instead of
  full rebuild (F5).** Depends on R8; replaces teardown-and-rebuild with
  targeted mutations. Largest performance win for big encounters; only
  worthwhile once R8 provides a single place to do it safely.

Suggested order: R1–R5 (safe, immediately valuable) → R6–R7 (contained) →
R8 → R9 (sequenced, planned).

## Next steps

- Land the small wins R1–R5 as an independent, low-risk hardening pass
  (they do not depend on the larger refactor and each is self-contained).
- Verify R3 against an all-grouped encounter (one group, no ungrouped rows):
  confirm "Next" cycles and that removing the active grouped combatant
  reassigns the active row correctly.
- Treat R8/R9 as a separate design effort; if pursued, dispatch the Design
  Agent with this file and
  `dev/investigations/grouped-combatant-tracker-map-switch-crash.md` as
  supporting context.
- Cleared within scope: the model ownership story (combatants owned by token
  layers, events/groups/effects owned by the model), the per-content update
  granularity (HP/conditions/visibility do not force full rebuilds), and the
  group widget's own `QPointer` member list — these are correct as-is.
