#ifndef RULEINITIATIVE_H
#define RULEINITIATIVE_H

#include <QObject>

class BattleDialogModelCombatant;

class RuleInitiative : public QObject
{
    Q_OBJECT
public:
    explicit RuleInitiative(QObject *parent = nullptr);

    virtual QString getInitiativeType();

    bool rollInitiative(QList<BattleDialogModelCombatant*>& combatants);
    void sortInitiative(QList<BattleDialogModelCombatant*>& combatants);
    void newRound(QList<BattleDialogModelCombatant*>& combatants);
    virtual bool compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b) = 0;

    // Roll an initiative value for a single combatant (no UI). Returns the
    // complete result the system would assign at the start of an encounter
    // — die plus any modifiers, fully ruleset-defined. Default rolls a d20
    // with no bonus; per-system subclasses encapsulate the actual dice and
    // modifiers (5e: d20 + dex mod; 2e: d10 + speed factor; etc.).
    virtual int rollInitiativeFor(const BattleDialogModelCombatant* combatant) const;

    // Resolve the combatant's initiative modifier from its underlying class /
    // character template. Lookup chain (first hit wins):
    //   1. "dmh:initiativeMod" — explicit ruleset-neutral modifier
    //   2. "dexterity" — 5e-style ability score, converted via floor((s-10)/2)
    //   3. 0
    // This lets non-D&D rulesets (Daggerheart, etc.) define their own modifier
    // without depending on a "dexterity" attribute, while keeping legacy 5e
    // bestiary data working.
    static int initiativeModFor(const BattleDialogModelCombatant* combatant);

protected:
    virtual bool preRollInitiative(QList<BattleDialogModelCombatant*>& combatants);
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) = 0;
    virtual bool postRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult);

    virtual void preSortInitiative(QList<BattleDialogModelCombatant*>& combatants);
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) = 0;
    virtual void postSortInitiative(QList<BattleDialogModelCombatant*>& combatants);

    virtual void internalNewRound(QList<BattleDialogModelCombatant*>& combatants);

    void resetCombatantSortValues(QList<BattleDialogModelCombatant*> combatants);

signals:
};

#endif // RULEINITIATIVE_H
