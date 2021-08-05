#include "undoshape.h"
#include "map.h"
#include "mapframe.h"

UndoShape::UndoShape(Map& map, const MapEditShape& mapEditShape) :
    UndoBase(map, QString("Paint")),
    _mapEditShape(mapEditShape)
{
}

void UndoShape::undo()
{
    _map.undoPaint();
    //if(_map.getRegisteredWindow())
    //    _map.getRegisteredWindow()->undoPaint();
}

void UndoShape::redo()
{
    /*
    if(_map.getRegisteredWindow())
    {
    */
    apply(true, nullptr);
    _map.updateFoW();
    /*
    if(_map.getRegisteredWindow())
    {
        _map.getRegisteredWindow()->updateFoW();
    }
    */
}

void UndoShape::apply(bool preview, QPaintDevice* target) const
{
    _map.paintFoWRect(_mapEditShape.rect(), _mapEditShape, target, preview);
}

QDomElement UndoShape::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
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

void UndoShape::inputXML(const QDomElement &element, bool isImport)
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

int UndoShape::getType() const
{
    return DMHelper::ActionType_Rect;
}

UndoBase* UndoShape::clone() const
{
    return new UndoShape(_map, _mapEditShape);
}

const MapEditShape& UndoShape::mapEditShape() const
{
    return _mapEditShape;
}

MapEditShape& UndoShape::mapEditShape()
{
    return _mapEditShape;
}

