#include "combatantfactory.h"
#include "character.h"
#include "monster.h"
#include "dmconstants.h"
#include "bestiary.h"
#include <QDomElement>
#include <QString>

CombatantFactory::CombatantFactory(QObject *parent) :
    QObject(parent)
{
}

Combatant* CombatantFactory::createCombatant(int combatantType, const QDomElement& element, QObject *parent)
{
    Combatant* combatant = NULL;

    switch(combatantType)
    {
        case DMHelper::CombatantType_Base:
            break;
        case DMHelper::CombatantType_Character:
            combatant = new Character(parent);
            break;
        case DMHelper::CombatantType_Monster:
            combatant = Bestiary::Instance()->createMonster(element);
            break;
        default:
            break;
    }

    if(combatant)
        combatant->inputXML(element);

    return combatant;
}
