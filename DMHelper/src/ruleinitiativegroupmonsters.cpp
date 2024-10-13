#include "ruleinitiativegroupmonsters.h"
#include "battledialogmodelcombatant.h"
#include "initiativelistcombatantwidget.h"
#include "initiativelistdialog.h"

QString RuleInitiativeGroupMonsters::InitiativeType = QString("group-monsters");
QString RuleInitiativeGroupMonsters::InitiativeDescription = QString("Group Initiative for Monsters");

RuleInitiativeGroupMonsters::RuleInitiativeGroupMonsters(QObject *parent)
    : RuleInitiative{parent}
{}

QString RuleInitiativeGroupMonsters::getInitiativeType()
{
    return RuleInitiativeGroupMonsters::InitiativeType;
}

bool RuleInitiativeGroupMonsters::compareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    return RuleInitiativeGroupMonsters::CompareCombatants(a, b);
}

bool RuleInitiativeGroupMonsters::internalRollInitiative(QList<BattleDialogModelCombatant*>& combatants, bool previousResult)
{
    if((combatants.isEmpty()) || (!previousResult))
        return false;

    InitiativeListDialog* dlg = new InitiativeListDialog();
    
    InitiativeListCombatantWidget* widgetMonsters = new InitiativeListCombatantWidget(0, QPixmap(":/img/data/icon_bestiary.png"), QString("Monsters"));
    dlg->addCombatantWidget(widgetMonsters);

    for(int i = 0; i < combatants.count(); ++i)
    {
        BattleDialogModelCombatant* combatant = combatants.at(i);
        if((combatant) && (combatant->getCombatantType() != DMHelper::CombatantType_Monster))
        {
            InitiativeListCombatantWidget* widget = new InitiativeListCombatantWidget(combatant);
            dlg->addCombatantWidget(widget);
        }
    }

    widgetMonsters->setInitiativeFocus();

    int result = dlg->exec();
    if(result == QDialog::Accepted)
    {
        // Go through the PC widgets and update their initiative values
        for(int i = 0; i < dlg->getCombatantCount(); ++i)
        {
            InitiativeListCombatantWidget* widget = dlg->getCombatantWidget(i);
            if((widget) && (widget != widgetMonsters))
            {
                BattleDialogModelCombatant* combatant = widget->getCombatant();
                if(combatant)
                    combatant->setInitiative(widget->getInitiative());
            }
        }

        // Go through the Monster combatants and update their initiative values
        for(int i = 0; i < combatants.count(); ++i)
        {
            BattleDialogModelCombatant* combatant = combatants.at(i);
            if((combatant) && (combatant->getCombatantType() == DMHelper::CombatantType_Monster))
                combatant->setInitiative(widgetMonsters->getInitiative());
        }
    }

    dlg->deleteLater();
    return (result == QDialog::Accepted);
}

void RuleInitiativeGroupMonsters::internalSortInitiative(QList<BattleDialogModelCombatant*>& combatants)
{
    std::sort(combatants.begin(), combatants.end(), CompareCombatants);
}

bool RuleInitiativeGroupMonsters::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
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
