#ifndef RULEINITIATIVE5E_H
#define RULEINITIATIVE5E_H

#include "ruleinitiative.h"

class RuleInitiative5e : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiative5e(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

    virtual bool rollInitiative(QList<BattleDialogModelCombatant*>& combatants) override;
    virtual void sortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);
};

#endif // RULEINITIATIVE5E_H