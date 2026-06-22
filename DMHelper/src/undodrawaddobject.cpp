#include "undodrawaddobject.h"
#include "layerdrawstate.h"

UndoDrawAddObject::UndoDrawAddObject(LayerDrawState* state, LayerDrawObject* object) :
    _state(state),
    _object(object),
    _objectId(_object ? _object->getId() : QUuid()),
    _index{-1}
{}

void UndoDrawAddObject::redo()
{
    if(!_state)
        return;

    _index = _state->appendObject(_object);
}

void UndoDrawAddObject::undo()
{
    if(!_state)
        return;

    _object = _state->takeObject(_objectId);
}
