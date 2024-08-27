#include "ruleinitiative2e.h"
#include "battledialogmodelcombatant.h"
#include "initiativelistcombatantwidget.h"
#include "initiativelistdialog.h"

QString RuleInitiative2e::InitiativeType = QString("2e");
QString RuleInitiative2e::InitiativeDescription = QString("D&D 2e Standard Initiative");

RuleInitiative2e::RuleInitiative2e(QObject *parent) :
    RuleInitiative{parent}
{}

QString RuleInitiative2e::getInitiativeType()
{
    return RuleInitiative2e::InitiativeType;
}

bool RuleInitiative2e::compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    return RuleInitiative2e::CompareCombatants(a, b);
}

bool RuleInitiative2e::internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    if((combatants.isEmpty()) || (!previousResult))
        return false;

    InitiativeListDialog* dlg = new InitiativeListDialog();
    InitiativeListCombatantWidget* firstWidget = nullptr;

    // First add all non-monsters
    for(int i = 0; i < combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = combatants.at(i);
        if((combatant) && (combatant->getCombatantType() != DMHelper::CombatantType_Monster))
        {
            InitiativeListCombatantWidget* widget = new InitiativeListCombatantWidget(combatant);
            dlg->addCombatantWidget(widget);
            if(!firstWidget)
                firstWidget = widget;
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
            if(!firstWidget)
                firstWidget = widget;
        }
    }

    if(firstWidget)
        firstWidget->setInitiativeFocus();

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

void RuleInitiative2e::internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    std::sort(combatants.begin(), combatants.end(), CompareCombatants);
}

void RuleInitiative2e::internalNewRound(QList<BattleDialogModelCombatant*>& combatants)
{
    rollInitiative(combatants);
}

bool RuleInitiative2e::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a)||(!b))
        return false;

    // Sort by lower initiative
    return a->getInitiative() < b->getInitiative();
}
