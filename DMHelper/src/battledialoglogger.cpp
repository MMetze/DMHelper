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

BattleDialogLogger::BattleDialogLogger(const BattleDialogLogger& other) :
    DMHObjectBase(other.parent()),
    _battleEvents()
{
    for(BattleDialogEvent* i : other._battleEvents)
    {
        _battleEvents.append(i->clone());
    }
}

BattleDialogLogger::~BattleDialogLogger()
{
    qDeleteAll(_battleEvents);
}

BattleDialogLogger& BattleDialogLogger::operator=(const BattleDialogLogger& other)
{
    qDeleteAll(_battleEvents);
    _battleEvents.clear();

    for(BattleDialogEvent* i : other._battleEvents)
    {
        _battleEvents.append(i->clone());
    }

    return *this;
}

QDomElement BattleDialogLogger::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
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

    return loggerElement;
}

void BattleDialogLogger::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.isNull())
        return;

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

QList<BattleDialogEvent*> BattleDialogLogger::getEvents() const
{
    return _battleEvents;
}

int BattleDialogLogger::getRounds() const
{
    int rounds = 1;
    for(BattleDialogEvent* event : _battleEvents)
    {
        if(event->getType() == DMHelper::BattleEvent_NewRound)
            ++rounds;
    }
    return rounds;
}

void BattleDialogLogger::damageDone(QUuid combatantID, QUuid targetID, int damage)
{
    _battleEvents.append(new BattleDialogEventDamage(combatantID, targetID, damage));
}

void BattleDialogLogger::newRound()
{
    _battleEvents.append(new BattleDialogEventNewRound());
    emit roundsChanged();
}
