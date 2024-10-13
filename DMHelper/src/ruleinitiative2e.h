#ifndef RULEINITIATIVE2E_H
#define RULEINITIATIVE2E_H

#include "ruleinitiative.h"

class RuleInitiative2e : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiative2e(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

    virtual bool compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b) override;

protected:
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) override;
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;
    virtual void internalNewRound(QList<BattleDialogModelCombatant*>& combatants) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);

};

#endif // RULEINITIATIVE2E_H
