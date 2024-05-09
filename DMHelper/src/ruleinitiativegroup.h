#ifndef RULEINITIATIVEGROUP_H
#define RULEINITIATIVEGROUP_H

#include "ruleinitiative.h"

class RuleInitiativeGroup : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiativeGroup(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

protected:
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) override;
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);
};

#endif // RULEINITIATIVEGROUP_H
