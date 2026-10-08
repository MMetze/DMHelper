#ifndef RULEHEALTH_H
#define RULEHEALTH_H

#include <QObject>
#include <QString>

class BattleDialogModelCombatant;
class MonsterClassv2;
class TemplateObject;

// Per-ruleset interface that owns "what does damage do", "is this combatant
// dead", and "what HP does a freshly-spawned combatant have". Parallel to
// RuleInitiative. Resource-backed tracks use the ruleset's template key;
// scalar tracks retain the legacy HP accessors and DMH_KEY_HEALTH override.
//
// Damage direction is ruleset-defined: 5e counts down (start at max, dead
// at <= 0); Daggerheart counts up (start at 0, dead at >= max).
class RuleHealth : public QObject
{
    Q_OBJECT
public:
    explicit RuleHealth(QObject* parent = nullptr);

    virtual QString getHealthType() const;

    // Resource tracks read current/max from the template-key override, falling
    // back to the source resource. Scalar tracks read DMH_KEY_HEALTH or legacy HP.
    virtual int getHealth(const BattleDialogModelCombatant* combatant) const;
    void setHealth(BattleDialogModelCombatant* combatant, int value);
    QString resourceHealthKey(const BattleDialogModelCombatant* combatant) const;

    // Read the combatant's max health (for clamping and isDead in count-up
    // rulesets). Resource tracks use the same pair as getHealth; scalar monster
    // tracks retain rolled per-instance maxima. Returns 0 without a source.
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

    // Returns a health fraction in [0, 1] where 1.0 means fully healthy and
    // 0.0 means dead/at-limit. Interpretation depends on the ruleset:
    // count-down (5e) uses current/max; count-up (Daggerheart) inverts it.
    // Returns 0.0 when max <= 0 (guard against division by zero).
    virtual qreal getHealthFraction(const BattleDialogModelCombatant* combatant) const;

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
    static bool combatantIsDead(const BattleDialogModelCombatant* combatant);
    // Visual-only: preserve explicit unconscious conditions and grey PCs
    // whose health track reaches the ruleset's incapacitation threshold.
    static bool combatantIsUnconscious(const BattleDialogModelCombatant* combatant);

protected:
    // Helper: resolve the underlying template object (MonsterClassv2 for monsters,
    // Characterv2 for PCs) or nullptr. Mirrors the RuleInitiative::initiativeModFor
    // lookup pattern.
    static TemplateObject* templateFor(const BattleDialogModelCombatant* combatant);

    // Helper: return the ruleset-configured template key for max HP for the
    // given combatant (character or monster). Falls back to hard-coded
    // defaults when the combatant is not attached to a campaign.
    QString maxHpKeyFor(const BattleDialogModelCombatant* combatant) const;
    QString currentHpKeyFor(const BattleDialogModelCombatant* combatant) const;
};

#endif // RULEHEALTH_H
