#ifndef RULEINITIATIVEGROUPMONSTERS_H
#define RULEINITIATIVEGROUPMONSTERS_H

#include "ruleinitiative.h"

class RuleInitiativeGroupMonsters : public RuleInitiative
{
    Q_OBJECT
public:
    explicit RuleInitiativeGroupMonsters(QObject *parent = nullptr);

    static QString InitiativeType;
    static QString InitiativeDescription;

    virtual QString getInitiativeType() override;

    virtual bool compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b) override;

protected:
    virtual bool internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult) override;
    virtual void internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants) override;

private:
    static bool CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b);

};

#endif // RULEINITIATIVEGROUPMONSTERS_H
