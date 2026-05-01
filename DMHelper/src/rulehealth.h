#ifndef RULEHEALTH_H
#define RULEHEALTH_H

#include <QObject>
#include <QString>

class BattleDialogModelCombatant;
class MonsterClassv2;
class TemplateObject;

// Per-ruleset interface that owns "what does damage do", "is this combatant
// dead", and "what HP does a freshly-spawned combatant have". Parallel to
// RuleInitiative. The primary health track is stored on the combatant under
// the canonical override key BattleDialogModelCombatant::DMH_KEY_HEALTH.
//
// Damage direction is ruleset-defined: 5e counts down (start at max, dead
// at <= 0); Daggerheart counts up (start at 0, dead at >= max).
class RuleHealth : public QObject
{
    Q_OBJECT
public:
    explicit RuleHealth(QObject* parent = nullptr);

    virtual QString getHealthType() const;

    // Read the combatant's current primary-track health value. Defaults to
    // reading DMH_KEY_HEALTH override, falling back to the legacy
    // getHitPoints() accessor when the override is absent.
    virtual int getHealth(const BattleDialogModelCombatant* combatant) const;

    // Read the combatant's max health (for clamping and isDead in count-up
    // rulesets). Default reads "hit_points" off the combatant's source
    // template (MonsterClassv2 / Characterv2). Returns 0 when no source is
    // available.
    virtual int getMaxHealth(const BattleDialogModelCombatant* combatant) const;

    // Apply a signed damage delta. Positive = damage, negative = healing.
    // The interpretation of "damage" (subtractive vs additive on the stored
    // health value) is ruleset-defined. Returns the new stored value.
    virtual int applyDamage(BattleDialogModelCombatant* combatant, int amount) = 0;

    // True when the combatant has been removed from play (cannot be revived
    // by ordinary means). Drives "remove dead" sweeps and dead-tile styling.
    virtual bool isDead(const BattleDialogModelCombatant* combatant) const = 0;

    // True when the combatant is unable to take actions (typically same as
    // isDead in systems without a separate downed state).
    virtual bool isIncapacitated(const BattleDialogModelCombatant* combatant) const = 0;

    // Capability flag: does this ruleset have a death-save mechanic that the
    // UI should expose? Default false.
    virtual bool hasDeathSaves() const;

    // Direction the primary health track moves on damage. False = subtract
    // (5e), true = add (Daggerheart). UI uses this for damage popup colour
    // cues and keypad direction hints.
    virtual bool healthCountsUp() const = 0;

    // Initialise the combatant's primary-track health when it is first spawned
    // (e.g. from a bestiary entry). Stores the result via setOverride on the
    // combatant. Implementations consult the source MonsterClassv2 / Characterv2
    // via dynamic_cast on the combatant.
    virtual void rollInitial(BattleDialogModelCombatant* combatant) = 0;

    // Convenience: walk the combatant's parent chain to find an enclosing
    // Campaign and return its ruleset's RuleHealth, or nullptr when the
    // combatant is not attached to a campaign. Lets callers that only hold a
    // combatant pointer (dialogs, popup widgets) participate in the new
    // ruleset-aware damage path without plumbing a Ruleset reference through.
    static RuleHealth* forCombatant(const BattleDialogModelCombatant* combatant);

protected:
    // Helper: resolve the underlying template object (MonsterClassv2 for monsters,
    // Characterv2 for PCs) or nullptr. Mirrors the RuleInitiative::initiativeModFor
    // lookup pattern.
    static TemplateObject* templateFor(const BattleDialogModelCombatant* combatant);
};

#endif // RULEHEALTH_H
