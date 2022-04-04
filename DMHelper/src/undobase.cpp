#include "undobase.h"
#include "map.h"
#include "dmconstants.h"

UndoBase::UndoBase(Map* map, const QString & text) :
    QUndoCommand(text),
    _map(map),
    _removed(false)
{
}

UndoBase::~UndoBase()
{
}

bool UndoBase::isRemoved() const
{
    return _removed;
}

void UndoBase::setRemoved(bool removed)
{
    _removed = removed;
}

int UndoBase::getType() const
{
    return DMHelper::ActionType_Base;
}

void UndoBase::setMap(Map* map)
{
    _map = map;
}
