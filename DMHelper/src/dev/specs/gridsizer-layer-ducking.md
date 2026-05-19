# GridSizer Layer Ducking

## Summary
When the grid-sizer overlay is active on a MapFrame or BattleFrame,
all LayerType_Grid layers must be temporarily hidden on the DM View
so the user can see the underlying map while adjusting grid size.
Once the grid-sizer is dismissed (whether accepted or cancelled) the
grid layers must be restored to exactly the visibility state they had
before the grid-sizer appeared. No serialised data must change.

## User-visible behaviour
- User clicks "resize grid" (or equivalent menu/toolbar action).
- Any visible LayerType_Grid layers disappear from the DM View
  immediately when the GridSizer widget appears.
- The user drags the GridSizer to size the grid, then accepts or
  cancels.
- All LayerType_Grid layers that were visible before the GridSizer
  appeared become visible again. Layers that were already hidden
  remain hidden.
- The campaign is not marked dirty by this operation.
- The player view is unaffected (player grid visibility is not changed).
- Works identically on both MapFrame and BattleFrame.

## Subsystems
- Battle (BattleFrame)
- UI shell (MapFrame)
- Layer system (LayerScene, LayerGrid)

## Done conditions
- Opening the GridSizer hides all DM-visible LayerType_Grid layers.
- Accepting the GridSizer restores them to their pre-sizer state.
- Cancelling the GridSizer restores them to their pre-sizer state.
- A layer that was hidden before the GridSizer appeared stays hidden
  after dismissal.
- No dirty() signal is emitted as a result of the hide/restore cycle.
- Build succeeds with no new errors.

## Open questions
None.
