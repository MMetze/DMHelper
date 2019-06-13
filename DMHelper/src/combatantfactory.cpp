#include "combatantfactory.h"
#include "character.h"
#include "monster.h"
#include "dmconstants.h"
#include "bestiary.h"
#include "combatantreference.h"
#include <QDomElement>
#include <QString>

CombatantFactory::CombatantFactory(QObject *parent) :
    QObject(parent)
{
}

Combatant* CombatantFactory::createCombatant(int combatantType, const QDomElement& element, bool isImport, QObject *parent)
{
    Combatant* combatant = nullptr;

    switch(combatantType)
    {
        case DMHelper::CombatantType_Base:
            break;
        case DMHelper::CombatantType_Character:
            combatant = new Character(parent);
            break;
        case DMHelper::CombatantType_Monster:
            combatant = Bestiary::Instance()->createMonster(element, isImport);
            break;
        case DMHelper::CombatantType_Reference:
            combatant = new CombatantReference(parent);
            break;
        default:
            break;
    }

    if(combatant)
        combatant->inputXML(element, isImport);

    return combatant;
}
