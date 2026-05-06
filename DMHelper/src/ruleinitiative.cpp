#include "ruleinitiative.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelcharacter.h"
#include "monsterclassv2.h"
#include "characterv2.h"
#include "templateobject.h"
#include "dice.h"

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

int RuleInitiative::rollInitiativeFor(const BattleDialogModelCombatant* combatant) const
{
    Q_UNUSED(combatant);
    return Dice::d20();
}

int RuleInitiative::initiativeModFor(const BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return 0;

    // Reach the underlying template object that holds attribute data. The
    // combatant model is a thin per-encounter wrapper; the actual stat sheet
    // is on the MonsterClassv2 (for monsters) or Characterv2 (for PCs).
    TemplateObject* tmpl = nullptr;
    if(const BattleDialogModelMonsterBase* mb = dynamic_cast<const BattleDialogModelMonsterBase*>(combatant))
        tmpl = mb->getMonsterClass();
    else if(const BattleDialogModelCharacter* cm = dynamic_cast<const BattleDialogModelCharacter*>(combatant))
        tmpl = cm->getCharacter();

    if(!tmpl)
        return 0;

    static const QString KEY_INIT_MOD = QStringLiteral("dmh:initiativeMod");
    static const QString KEY_DEXTERITY = QStringLiteral("dexterity");

    if(tmpl->hasValue(KEY_INIT_MOD))
        return tmpl->getIntValue(KEY_INIT_MOD);

    if(tmpl->hasValue(KEY_DEXTERITY))
    {
        // 5e ability modifier: floor((score - 10) / 2). C++ integer division
        // truncates toward zero, so the negative-score branch needs an explicit
        // floor (e.g. dex 9 -> -1, not 0).
        const int dex = tmpl->getIntValue(KEY_DEXTERITY);
        return (dex >= 10) ? (dex - 10) / 2 : -((11 - dex) / 2);
    }

    return 0;
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

