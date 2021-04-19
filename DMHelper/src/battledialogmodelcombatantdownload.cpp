#include "battledialogmodelcombatantdownload.h"
#include "combatantdownload.h"

BattleDialogModelCombatantDownload::BattleDialogModelCombatantDownload() :
    BattleDialogModelCombatant()
{
}

int BattleDialogModelCombatantDownload::getCombatantType() const
{
    return DMHelper::CombatantType_Download;
}

BattleDialogModelCombatant* BattleDialogModelCombatantDownload::clone() const
{
    return nullptr;
}

qreal BattleDialogModelCombatantDownload::getSizeFactor() const
{
    if(_combatant)
    {
        CombatantDownload* combatantDownload = dynamic_cast<CombatantDownload*>(_combatant);
        if(combatantDownload)
            return combatantDownload->getSizeFactor();
    }

    return 1.0;
}

int BattleDialogModelCombatantDownload::getSizeCategory() const
{
    return DMHelper::CombatantSize_Unknown;
}

int BattleDialogModelCombatantDownload::getStrength() const
{
    if(_combatant)
        return _combatant->getStrength();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getDexterity() const
{
    if(_combatant)
        return _combatant->getDexterity();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getConstitution() const
{
    if(_combatant)
        return _combatant->getConstitution();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getIntelligence() const
{
    if(_combatant)
        return _combatant->getIntelligence();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getWisdom() const
{
    if(_combatant)
        return _combatant->getWisdom();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getCharisma() const
{
    if(_combatant)
        return _combatant->getCharisma();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getSkillModifier(Combatant::Skills skill) const
{
    Q_UNUSED(skill);
    return 0;
}

bool BattleDialogModelCombatantDownload::hasCondition(Combatant::Condition condition) const
{
    if(_combatant)
        return _combatant->hasCondition(condition);
    else
        return false;
}

int BattleDialogModelCombatantDownload::getSpeed() const
{
    if(_combatant)
        return _combatant->getSpeed();
    else
        return 30;
}

int BattleDialogModelCombatantDownload::getArmorClass() const
{
    if(_combatant)
        return _combatant->getArmorClass();
    else
        return 10;
}

int BattleDialogModelCombatantDownload::getHitPoints() const
{
    if(_combatant)
        return _combatant->getHitPoints();
    else
        return 1;
}

void BattleDialogModelCombatantDownload::setHitPoints(int hitPoints)
{
    if(_combatant)
        _combatant->setHitPoints(hitPoints);
}

QPixmap BattleDialogModelCombatantDownload::getIconPixmap(DMHelper::PixmapSize iconSize) const
{
    // TODO: need this?
    Q_UNUSED(iconSize);
    return QPixmap();
}

void BattleDialogModelCombatantDownload::setConditions(int conditions)
{
    if(_combatant)
        _combatant->setConditions(conditions);
}

void BattleDialogModelCombatantDownload::applyConditions(int conditions)
{
    if(_combatant)
        _combatant->applyConditions(conditions);
}

void BattleDialogModelCombatantDownload::removeConditions(int conditions)
{
    if(_combatant)
        _combatant->removeConditions(conditions);
}
