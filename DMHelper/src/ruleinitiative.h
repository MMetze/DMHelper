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
