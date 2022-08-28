#include "undofowbase.h"
#include "layerfow.h"
#include "dmconstants.h"

UndoFowBase::UndoFowBase(LayerFow* layer, const QString & text) :
    QUndoCommand(text),
    _layer(layer),
    _removed(false)
{
}

UndoFowBase::~UndoFowBase()
{
}

bool UndoFowBase::isRemoved() const
{
    return _removed;
}

void UndoFowBase::setRemoved(bool removed)
{
    _removed = removed;
}

int UndoFowBase::getType() const
{
    return DMHelper::ActionType_Base;
}

void UndoFowBase::setLayer(LayerFow* layer)
{
    _layer = layer;
}
