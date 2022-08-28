#include "undofowshape.h"
#include "layerfow.h"

UndoFowShape::UndoFowShape(LayerFow* layer, const MapEditShape& mapEditShape) :
    UndoFowBase(layer, QString("Paint")),
    _mapEditShape(mapEditShape)
{
}

void UndoFowShape::undo()
{
    if(_layer)
       _layer->undoPaint();
}

void UndoFowShape::redo()
{
    if(_layer)
    {
        apply(true, nullptr);
        // TODO?
        //_layer->updateFoW();
    }
}

void UndoFowShape::apply(bool preview, QPaintDevice* target) const
{
    if(_layer)
        _layer->paintFoWRect(_mapEditShape.rect(), _mapEditShape, target, preview);
}

QDomElement UndoFowShape::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("x", _mapEditShape.rect().x());
    element.setAttribute("y", _mapEditShape.rect().y());
    element.setAttribute("width", _mapEditShape.rect().width());
    element.setAttribute("height", _mapEditShape.rect().height());
    element.setAttribute("erase", static_cast<int>(_mapEditShape.erase()));
    element.setAttribute("smooth", static_cast<int>(_mapEditShape.smooth()));

    return element;
}

void UndoFowShape::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    QRect inputRect;
    inputRect.setX(element.attribute(QString("x")).toInt());
    inputRect.setY(element.attribute(QString("y")).toInt());
    inputRect.setWidth(element.attribute(QString("width")).toInt());
    inputRect.setHeight(element.attribute(QString("height")).toInt());
    _mapEditShape.setRect(inputRect);
    _mapEditShape.setErase(static_cast<bool>(element.attribute("erase", QString::number(1)).toInt()));
    _mapEditShape.setSmooth(static_cast<bool>(element.attribute("smooth", QString::number(1)).toInt()));
}

int UndoFowShape::getType() const
{
    return DMHelper::ActionType_Rect;
}

UndoFowBase* UndoFowShape::clone() const
{
    return new UndoFowShape(_layer, _mapEditShape);
}

const MapEditShape& UndoFowShape::mapEditShape() const
{
    return _mapEditShape;
}

MapEditShape& UndoFowShape::mapEditShape()
{
    return _mapEditShape;
}

