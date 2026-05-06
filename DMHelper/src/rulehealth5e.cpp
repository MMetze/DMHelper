#include "rulehealth5e.h"
#include "battledialogmodelcombatant.h"
#include "battledialogmodelmonsterbase.h"
#include "battledialogmodelmonstercombatant.h"
#include "monsterclassv2.h"
#include "templateobject.h"
#include "dice.h"

// 5e clamps damage taken so health never falls below zero in the stored value;
// healing above max is the responsibility of higher-level logic (the same as
// the pre-refactor behaviour in BattleFrame).
static const int HEALTH_FLOOR_5E = 0;
static const QString KEY_HIT_DICE_5E = QStringLiteral("hit_dice");

QString RuleHealth5e::HealthType = QString("5e");
QString RuleHealth5e::HealthDescription = QString("D&D 5e Hit Points (count down, dead at 0)");

RuleHealth5e::RuleHealth5e(QObject* parent) :
    RuleHealth{parent}
{
}

QString RuleHealth5e::getHealthType() const
{
    return RuleHealth5e::HealthType;
}

int RuleHealth5e::applyDamage(BattleDialogModelCombatant* combatant, int amount)
{
    if(!combatant)
        return 0;

    const int current = getHealth(combatant);
    int updated = current - amount;
    if(updated < HEALTH_FLOOR_5E)
        updated = HEALTH_FLOOR_5E;

    // Route through the typed setter so legacy mirrors (_monsterHP, etc.) and
    // the override bag stay in sync. setHitPoints emits dataChanged and also
    // writes the DMH_KEY_HEALTH override.
    combatant->setHitPoints(updated);
    return updated;
}

bool RuleHealth5e::isDead(const BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return false;

    return getHealth(combatant) <= HEALTH_FLOOR_5E;
}

bool RuleHealth5e::isIncapacitated(const BattleDialogModelCombatant* combatant) const
{
    // No death-save UI exists today; treat incapacitated identically to dead.
    return isDead(combatant);
}

bool RuleHealth5e::hasDeathSaves() const
{
    // Forward-compatible: 5e supports death saves; no UI consumes this yet.
    return true;
}

bool RuleHealth5e::healthCountsUp() const
{
    return false;
}

void RuleHealth5e::rollInitial(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    const TemplateObject* tmpl = templateFor(combatant);
    if(!tmpl)
        return;

    int rolled = 0;
    if(tmpl->hasValue(KEY_HIT_DICE_5E))
    {
        Dice hitDice = tmpl->getDiceValue(KEY_HIT_DICE_5E);
        rolled = hitDice.roll();
    }

    const QString hpKey = maxHpKeyFor(combatant);
    if((rolled == 0) && (tmpl->hasValue(hpKey)))
        rolled = tmpl->getIntValue(hpKey);

    combatant->setHitPoints(rolled);

    BattleDialogModelMonsterCombatant* monsterCombatant = dynamic_cast<BattleDialogModelMonsterCombatant*>(combatant);
    if(monsterCombatant)
        monsterCombatant->setMonsterMaxHP(rolled);
}
