#ifndef UNDODRAWADDOBJECT_H
#define UNDODRAWADDOBJECT_H

#include <QUndoCommand>
#include <QUuid>

class LayerDrawState;
class LayerDrawObject;

class UndoDrawAddObject : public QUndoCommand
{
public:
    UndoDrawAddObject(LayerDrawState* state, LayerDrawObject* object);

    virtual void redo() override;
    virtual void undo() override;

protected:
    LayerDrawState* _state;
    LayerDrawObject* _object;
    QUuid _objectId;
    int _index;
};

#endif // UNDODRAWADDOBJECT_H
