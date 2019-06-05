#include "battledialoglogger.h"
#include "battledialogevent.h"
#include "battledialogeventnewround.h"
#include "battledialogeventdamage.h"
#include "dmconstants.h"
#include <QDomDocument>

BattleDialogLogger::BattleDialogLogger(QObject *parent) :
    DMHObjectBase(parent),
    _battleEvents()
{
}

BattleDialogLogger::~BattleDialogLogger()
{
    qDeleteAll(_battleEvents);
}

void BattleDialogLogger::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    QDomElement loggerElement = doc.createElement( "battlelogger" );

    DMHObjectBase::outputXML(doc, loggerElement, targetDirectory, isExport);

    for(BattleDialogEvent* i : _battleEvents)
    {
        QDomElement eventElement = doc.createElement( "battleevent");
        eventElement.setAttribute( "type", i->getType());
        i->outputXML(eventElement, isExport);
        loggerElement.appendChild(eventElement);
    }

    parent.appendChild(loggerElement);
}

void BattleDialogLogger::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    qDeleteAll(_battleEvents);
    _battleEvents.clear();

    QDomElement eventElement = element.firstChildElement("battleevent");
    while(!eventElement.isNull())
    {
        int eventType = eventElement.attribute("type",QString::number(DMHelper::BattleEvent_Blank)).toInt();
        switch(eventType)
        {
            case DMHelper::BattleEvent_Damage:
                _battleEvents.append(new BattleDialogEventDamage(eventElement));
                break;
            case DMHelper::BattleEvent_NewRound:
                _battleEvents.append(new BattleDialogEventNewRound(eventElement));
                break;
            case DMHelper::BattleEvent_Blank:
            default:
                break;
        }
        eventElement = eventElement.nextSiblingElement("battleevent");
    }
}

void BattleDialogLogger::postProcessXML(const QDomElement &element, bool isImport)
{
    DMHObjectBase::postProcessXML(element, isImport);
}

QList<BattleDialogEvent*> BattleDialogLogger::getEvents() const
{
    return _battleEvents;
}

void BattleDialogLogger::damageDone(QUuid combatantID, QUuid targetID, int damage)
{
    _battleEvents.append(new BattleDialogEventDamage(combatantID, targetID, damage));
}

void BattleDialogLogger::newRound()
{
    _battleEvents.append(new BattleDialogEventNewRound());
}
