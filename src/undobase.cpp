#include "undobase.h"
#include "map.h"
#include "dmconstants.h"

UndoBase::UndoBase(Map& map, const QString & text) :
    QUndoCommand(text),
    _map(map)
{
}

int UndoBase::getType() const
{
    return DMHelper::ActionType_Base;
}
