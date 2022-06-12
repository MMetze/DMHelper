#include "battledialogeventnewround.h"
#include "dmconstants.h"
#include <QDomElement>

BattleDialogEventNewRound::BattleDialogEventNewRound() :
    BattleDialogEvent()
{
}

BattleDialogEventNewRound::BattleDialogEventNewRound(const QDomElement& element) :
    BattleDialogEvent(element)
{
}

BattleDialogEventNewRound::BattleDialogEventNewRound(const BattleDialogEventNewRound& other) :
    BattleDialogEvent(other)
{
}

BattleDialogEventNewRound::~BattleDialogEventNewRound()
{
}

int BattleDialogEventNewRound::getType() const
{
    return DMHelper::BattleEvent_NewRound;
}

BattleDialogEvent* BattleDialogEventNewRound::clone()
{
    return new BattleDialogEventNewRound(*this);
}

QDomElement BattleDialogEventNewRound::outputXML(QDomElement &element, bool isExport)
{
    Q_UNUSED(isExport);
    return element;
}
