#include "ruleinitiative2e.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "templateobject.h"
#include "initiativelistcombatantwidget.h"
#include "initiativelistdialog.h"
#include "dice.h"

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

int RuleInitiative2e::rollInitiativeFor(const BattleDialogModelCombatant* combatant) const
{
    // AD&D 2e initiative: 1d10 + weapon speed factor; LOWER total goes first
    // (see CompareCombatants). Weapon speed factor lives on the underlying
    // character / monster sheet under "weaponSpeed" \u2014 unarmed / unequipped
    // entries default to 0 (no slow-down). Per RAW the Dexterity Reaction
    // Adjustment does NOT modify the initiative roll — it only affects
    // surprise checks and missile-attack accuracy — so it is intentionally
    // not applied here. (Optional 2e rules add casting time to spells, which
    // also requires per-attack context the rules engine does not yet expose.)
    if(!combatant)
        return Dice::d10();

    TemplateObject* tmpl = nullptr;
    if(const BattleDialogModelMonsterBase* mb = dynamic_cast<const BattleDialogModelMonsterBase*>(combatant))
        tmpl = mb->getMonsterClass();
    else if(const BattleDialogModelCharacter* cm = dynamic_cast<const BattleDialogModelCharacter*>(combatant))
        tmpl = cm->getCharacter();

    int weaponSpeed = 0;
    if(tmpl)
    {
        static const QString KEY_WEAPON_SPEED = QStringLiteral("weaponSpeed");
        if(tmpl->hasValue(KEY_WEAPON_SPEED))
            weaponSpeed = tmpl->getIntValue(KEY_WEAPON_SPEED);
    }

    return Dice::d10() + weaponSpeed;
}

bool RuleInitiative2e::CompareCombatants(const BattleDialogModelCombatant* a, const BattleDialogModelCombatant* b)
{
    if((!a)||(!b))
        return false;

    // Sort by lower initiative
    return a->getInitiative() < b->getInitiative();
}
