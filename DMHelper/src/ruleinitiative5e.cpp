#include "ruleinitiative5e.h"
#include "battledialogmodelcombatant.h"
#include "initiativelistcombatantwidget.h"
#include "initiativelistdialog.h"

QString RuleInitiative5e::InitiativeType = QString("5e");
QString RuleInitiative5e::InitiativeDescription = QString("D&D 5e Standard Initiative");

RuleInitiative5e::RuleInitiative5e(QObject *parent) :
    RuleInitiative{parent}
{}

QString RuleInitiative5e::getInitiativeType()
{
    return RuleInitiative5e::InitiativeType;
}

bool RuleInitiative5e::rollInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    if(combatants.isEmpty())
        return false;

    InitiativeListDialog* dlg = new InitiativeListDialog();

    // First add all non-monsters
    for(int i = 0; i < combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = combatants.at(i);
        if((combatant) && (combatant->getCombatantType() != DMHelper::CombatantType_Monster))
        {
            InitiativeListCombatantWidget* widget = new InitiativeListCombatantWidget(combatant);
            dlg->addCombatantWidget(widget);
        }
    }

    // Now add all non-monsters
    for(int i = 0; i < combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = combatants.at(i);
        if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
        {
            InitiativeListCombatantWidget* widget = new InitiativeListCombatantWidget(combatant);
            dlg->addCombatantWidget(widget);
        }
    }

    int result = dlg->exec();
    if(result == QDialog::Accepted)
    {
        for(int i = 0; i < dlg->getCombatantCount(); ++i)
        {
            InitiativeListCombatantWidget* widget = dlg->getCombatantWidget(i);
            if(widget)
            {
                BattleDialogModelCombatant* combatant = widget->getCombatant();
                if(combatant)
                    combatant->setInitiative(widget->getInitiative());
            }
        }
    }

    dlg->deleteLater();
    return (result == QDialog::Accepted);
}

void RuleInitiative5e::sortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    std::sort(combatants.begin(), combatants.end(), CompareCombatants);
    resetCombatantSortValues(combatants);
}

bool RuleInitiative5e::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a)||(!b))
        return false;

    // Sort by initiative first, then dexterity
    if(a->getInitiative() == b->getInitiative())
        return a->getDexterity() > b->getDexterity();
    else
        return a->getInitiative() > b->getInitiative();
}

