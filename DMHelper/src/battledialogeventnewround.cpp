#include "battledialogeventnewround.h"
#include "dmconstants.h"
#include <QDebug>

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

void BattleDialogEventNewRound::outputXML(QDomElement &element, bool isExport)
{
    Q_UNUSED(element);
    Q_UNUSED(isExport);
}
