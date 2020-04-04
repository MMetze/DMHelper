#include "combatantfactory.h"
#include "character.h"
#include "monster.h"
#include "dmconstants.h"
#include "bestiary.h"
#include "combatantreference.h"
#include <QDomElement>
#include <QString>
#include <QDebug>

CombatantFactory::CombatantFactory(QObject *parent) :
    ObjectFactory(parent)
{
}

Combatant* CombatantFactory::createCombatant(int combatantType, const QDomElement& element, bool isImport, QObject *parent)
{
    Combatant* combatant = nullptr;

    switch(combatantType)
    {
        case DMHelper::CombatantType_Monster:
            combatant = Bestiary::Instance()->createMonster(element, isImport);
            break;
        case DMHelper::CombatantType_Reference:
            combatant = new CombatantReference(QString(), parent);
            break;
        case DMHelper::CombatantType_Base:
        case DMHelper::CombatantType_Character:
            //combatant = new Character(parent);
            //break;
        default:
            qDebug() << "[CombatantFactory] ERROR: unexpected creation of battle combatant type: " << combatantType;
            break;
   }

    if(combatant)
        combatant->inputXML(element, isImport);

    return combatant;
}

CampaignObjectBase* CombatantFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);

    if(objectType != DMHelper::CampaignType_Combatant)
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected creation of non-combatant object type: " << objectType << ", subType: " << subType;
        return nullptr;
    }

    switch(objectType)
    {
        case DMHelper::CombatantType_Character:
            return new Character(objectName);
        case DMHelper::CombatantType_Reference:
            return new CombatantReference();
        case DMHelper::CombatantType_Base:
        case DMHelper::CombatantType_Monster:
        default:
            return nullptr;
    }
}

CampaignObjectBase* CombatantFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() != QString("combatant"))
        return nullptr;

    bool ok = false;
    int combatantType = element.attribute("type").toInt(&ok);

    if((!ok) || (combatantType != DMHelper::CombatantType_Character))
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected combatant type found reading element: " << combatantType << ", read success: " << ok;
        return nullptr;
    }

    return new Character();
}
