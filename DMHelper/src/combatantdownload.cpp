#include "combatantdownload.h"

CombatantDownload::CombatantDownload(QObject *parent) :
    Combatant(QString(), parent),
    _sizeFactor(1.0)
{
}

CombatantDownload::~CombatantDownload()
{
}

Combatant* CombatantDownload::clone() const
{
    return nullptr;
}

int CombatantDownload::getSpeed() const
{
    // TODO: store a speed value
    return 30;
}

int CombatantDownload::getStrength() const
{
    return 10;
}

int CombatantDownload::getDexterity() const
{
    return 10;
}

int CombatantDownload::getConstitution() const
{
    return 10;
}

int CombatantDownload::getIntelligence() const
{
    return 10;
}

int CombatantDownload::getWisdom() const
{
    return 10;
}

int CombatantDownload::getCharisma() const
{
    return 10;
}

qreal CombatantDownload::getSizeFactor() const
{
    return _sizeFactor;
}


