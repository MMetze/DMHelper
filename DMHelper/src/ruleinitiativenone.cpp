#include "ruleinitiativenone.h"

QString RuleInitiativeNone::InitiativeType = QString("none");
QString RuleInitiativeNone::InitiativeDescription = QString("No Initiative");

RuleInitiativeNone::RuleInitiativeNone(QObject *parent) :
    RuleInitiative{parent}
{}

QString RuleInitiativeNone::getInitiativeType()
{
    return RuleInitiativeNone::InitiativeType;
}

bool RuleInitiativeNone::compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    Q_UNUSED(a);
    Q_UNUSED(b);
    return false;
}

bool RuleInitiativeNone::internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    Q_UNUSED(combatants);
    Q_UNUSED(previousResult);
    return false;
}

void RuleInitiativeNone::internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
}
