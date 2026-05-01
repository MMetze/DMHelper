#include "rulehealthdaggerheart.h"
#include "battledialogmodelcombatant.h"

// Daggerheart's health track counts UP from 0. A combatant at 0 is fully
// healthy; reaching the class's hit_points cap kills them. Healing reduces
// the stored value; both ends are clamped to keep arithmetic well-behaved.
static const int HEALTH_FLOOR_DH = 0;
static const int INITIAL_HEALTH_DH = 0;

QString RuleHealthDaggerheart::HealthType = QString("daggerheart");
QString RuleHealthDaggerheart::HealthDescription = QString("Daggerheart Hit Points (count up, dead at max)");

RuleHealthDaggerheart::RuleHealthDaggerheart(QObject* parent) :
    RuleHealth{parent}
{
}

QString RuleHealthDaggerheart::getHealthType() const
{
    return RuleHealthDaggerheart::HealthType;
}

int RuleHealthDaggerheart::applyDamage(BattleDialogModelCombatant* combatant, int amount)
{
    if(!combatant)
        return 0;

    const int current = getHealth(combatant);
    const int max = getMaxHealth(combatant);

    int updated = current + amount;
    if(updated < HEALTH_FLOOR_DH)
        updated = HEALTH_FLOOR_DH;
    if((max > 0) && (updated > max))
        updated = max;

    combatant->setHitPoints(updated);
    return updated;
}

bool RuleHealthDaggerheart::isDead(const BattleDialogModelCombatant* combatant) const
{
    if(!combatant)
        return false;

    const int max = getMaxHealth(combatant);
    if(max <= 0)
        return false;

    return getHealth(combatant) >= max;
}

bool RuleHealthDaggerheart::isIncapacitated(const BattleDialogModelCombatant* combatant) const
{
    return isDead(combatant);
}

bool RuleHealthDaggerheart::healthCountsUp() const
{
    return true;
}

void RuleHealthDaggerheart::rollInitial(BattleDialogModelCombatant* combatant)
{
    if(!combatant)
        return;

    combatant->setHitPoints(INITIAL_HEALTH_DH);
}
