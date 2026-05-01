#ifndef RULEINITIATIVENONE_H
#define RULEINITIATIVENONE_H

#include "ruleinitiative.h"

class RuleInitiativeNone : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiativeNone(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

    virtual bool compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b) override;

protected:
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) override;
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;
};

#endif // RULEINITIATIVENONE_H
