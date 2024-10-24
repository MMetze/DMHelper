#include "ruleinitiativegroup.h"
#include "battledialogmodelcombatant.h"
#include "initiativelistcombatantwidget.h"
#include "initiativelistdialog.h"

QString RuleInitiativeGroup::InitiativeType = QString("group");
QString RuleInitiativeGroup::InitiativeDescription = QString("Group Initiative for Players and Monsters");

RuleInitiativeGroup::RuleInitiativeGroup(QObject *parent)
    : RuleInitiative{parent}
{}

QString RuleInitiativeGroup::getInitiativeType()
{
    return RuleInitiativeGroup::InitiativeType;
}

bool RuleInitiativeGroup::compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    return RuleInitiativeGroup::CompareCombatants(a, b);
}

bool RuleInitiativeGroup::internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    if((combatants.isEmpty()) || (!previousResult))
        return false;

    InitiativeListDialog* dlg = new InitiativeListDialog();
    
    InitiativeListCombatantWidget* widgetPCs = new InitiativeListCombatantWidget(0, QPixmap(":/img/data/icon_contentcharacter.png"), QString("PCs"));
    dlg->addCombatantWidget(widgetPCs);

    InitiativeListCombatantWidget* widgetMonsters = new InitiativeListCombatantWidget(0, QPixmap(":/img/data/icon_bestiary.png"), QString("Monsters"));
    dlg->addCombatantWidget(widgetMonsters);

    widgetPCs->setInitiativeFocus();

    int result = dlg->exec();
    if(result == QDialog::Accepted)
    {
        for(int i = 0; i < combatants.count(); ++i)
        {
            BattleDialogModelCombatant* combatant = combatants.at(i);
            if(combatant)
                combatant->setInitiative((combatant->getCombatantType() == DMHelper::CombatantType_Monster) ? widgetMonsters->getInitiative() : widgetPCs->getInitiative());
        }
    }

    dlg->deleteLater();
    return (result == QDialog::Accepted);
}

void RuleInitiativeGroup::internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    std::sort(combatants.begin(), combatants.end(), CompareCombatants);
}

bool RuleInitiativeGroup::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a)||(!b))
        return false;

    // Sort by initiative first, then combatant type (PCs before Monsters), then dexterity
    if(a->getInitiative() == b->getInitiative())
    {
        if(a->getCombatantType() == b->getCombatantType())
            return a->getDexterity() > b->getDexterity();
        else
            return b->getCombatantType() == DMHelper::CombatantType_Monster; // return true if b is a monster ==> a is not
    }
    else
    {
        return a->getInitiative() > b->getInitiative();
    }
}
