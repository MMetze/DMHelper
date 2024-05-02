#include "ruleinitiative.h"
#include "battledialogmodelcombatant.h"

RuleInitiative::RuleInitiative(QObject *parent) :
    QObject{parent}
{}

QString RuleInitiative::getInitiativeType()
{
    return QString("");
}

bool RuleInitiative::rollInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
    return false;
}

void RuleInitiative::sortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
}

void RuleInitiative::resetCombatantSortValues(QList<BattleDialogModelCombatant*> combatants)
{
    for(int i = 0; i < combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = combatants.at(i);
        if(combatant)
            combatant->setSortPosition(i);
    }
}

