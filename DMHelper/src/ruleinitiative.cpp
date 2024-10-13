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
    bool result = preRollInitiative(combatants);
    result = internalRollInitiative(combatants, result) || result;
    result = postRollInitiative(combatants, result) || result;
    return result;
}

void RuleInitiative::sortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    preSortInitiative(combatants);
    internalSortInitiative(combatants);
    postSortInitiative(combatants);

    resetCombatantSortValues(combatants);
}

void RuleInitiative::newRound(QList<BattleDialogModelCombatant*>& combatants)
{
    internalNewRound(combatants);
}

bool RuleInitiative::preRollInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
    return true;
}

bool RuleInitiative::postRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    Q_UNUSED(combatants);
    Q_UNUSED(previousResult);
    return true;
}

void RuleInitiative::preSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
}

void RuleInitiative::postSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    Q_UNUSED(combatants);
}

void RuleInitiative::internalNewRound(QList<BattleDialogModelCombatant*>& combatants)
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

