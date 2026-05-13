# Prompt — Active group highlight on the battle map pixmap

## Context

DMHelper has two surfaces that show the current initiative holder:

1. **Initiative pane (the side list)** — `CombatantGroupWidget` and the per-row
   `CombatantWidget`. When the active combatant belongs to a group,
   `BattleFrame::setActiveCombatant` calls `groupWidget->setActive(true)` and
   `combatantWidget->setActive(true)` on the single active member. As of the
   refactor that produced this prompt, the group widget propagates the active
   highlight down to every member widget so the whole slot is visually marked
   (red header tint, red trunk/ticks, red collapse-arrow, red left-bar on each
   member entry).

2. **Battle map (the QGraphicsScene rendering)** — each combatant is drawn as
   a token on `BattleDialogGraphicsScene`. The currently active combatant gets
   a glow / active marker (see `_renderer->setActiveToken(...)` in
   `battleframe.cpp` and the active-token pixmap composited by
   `PublishGLBattleRenderer`).

Today, only the *single* active combatant token gets the active-token marker on
the map. When the active combatant belongs to a group, the other members in
that same initiative slot are visually identical to inactive tokens. This is
inconsistent with the initiative-pane behaviour, where the whole group slot is
highlighted.

## Goal

Mirror the "whole group is active" treatment from the initiative pane onto the
battle map pixmap. When the active combatant belongs to a group, every member
of that group should also receive an active-token marker (or a related visual
cue — see "Open design questions" below).

## Suggested entry points

- `BattleFrame::setActiveCombatant` (DMHelper/src/battleframe.cpp ~line 4570).
  This is the central choke point that already drives the per-widget
  `setActive` calls and is the natural place to also fan out a "group active"
  notification to the renderer.
- `BattleDialogModelCombatant::getGroupId()` — used to locate sibling members
  within the same group.
- `BattleDialogModelCombatantGroup::getMembers()` — returns the model
  combatants in the group.
- `PublishGLBattleRenderer::setActiveToken` and the active-marker pixmap
  composition path. The current API takes a single token; you'll likely need
  either:
  - a list-based variant (`setActiveTokens(QList<...>)`), or
  - a separate `setGroupActiveTokens(QList<...>)` channel that uses a
    distinct visual treatment to differentiate "the one whose turn it
    technically is" from "other members of the active group slot".

## Open design questions to settle with the user before implementing

1. **Same marker, or a softer one for siblings?** The initiative pane uses the
   identical red bar on every member. The map could either (a) reuse the same
   active-token glow on all members, or (b) keep the bright glow on the
   nominal active combatant and apply a dimmer/secondary marker to the rest.
2. **Performance.** The active-token composition runs every frame the
   publisher updates. For groups with many members (e.g. a swarm of 12
   skeletons) compositing 12 glows per frame may matter — confirm the
   publisher's render budget before rolling this out.
3. **How does this interact with `setSelectedCombatant`?** Selection currently
   uses a different colour/marker. Make sure the new group-active marker
   doesn't clobber the selection visual when the user clicks one member of
   the active group.
4. **Player view vs DM view.** Confirm whether the group highlight should be
   visible to players, or only on the DM screen. The single-combatant active
   token is currently visible on both; matching that for groups may reveal
   member positions you'd rather keep hidden.

## Out of scope

- Do not change the initiative-pane group highlight behaviour — that's
  already implemented and validated.
- Do not change the active-token PNG asset itself. If a new visual is needed,
  produce it as a separate asset and route it through a new renderer channel.

## Files likely to be touched

- `DMHelper/src/battleframe.cpp` (active-combatant fan-out)
- `DMHelper/src/publishglbattlerenderer.{h,cpp}` (renderer API surface)
- `DMHelper/src/battledialoggraphicsscene.{h,cpp}` (if scene-level marker
  changes are needed instead of / in addition to the renderer composite)

Add new files only if absolutely necessary, and remember to update the
explicit source list in `CMakeLists.txt` (no globbing).
