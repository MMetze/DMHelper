#include "battledialogevent.h"

BattleDialogEvent::BattleDialogEvent(QObject *parent) :
    QObject(parent)
{
}

BattleDialogEvent::BattleDialogEvent(const QDomElement& element, QObject *parent) :
    QObject(parent)
{
    Q_UNUSED(element);
}

BattleDialogEvent::BattleDialogEvent(const BattleDialogEvent& other) :
    QObject(other.parent())
{
}

BattleDialogEvent::~BattleDialogEvent()
{
}

void BattleDialogEvent::outputXML(QDomElement &element, bool isExport)
{
    Q_UNUSED(element);
    Q_UNUSED(isExport);
}
