#include "undofowpoint.h"
#include "layerfow.h"
#include "dmconstants.h"
#include <QDomElement>

UndoFowPoint::UndoFowPoint(LayerFow* layer, const MapDrawPoint& mapDrawPoint) :
    UndoFowBase(layer, QString("Paint")),
    _mapDrawPoint(mapDrawPoint)
{
}

void UndoFowPoint::apply() const
{
    if(_layer)
        _layer->paintFoWPoint(_mapDrawPoint.point(), _mapDrawPoint);
}

QDomElement UndoFowPoint::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("x", _mapDrawPoint.point().x());
    element.setAttribute("y", _mapDrawPoint.point().y());
    element.setAttribute("radius", _mapDrawPoint.radius());
    element.setAttribute("brushtype", _mapDrawPoint.brushType());
    element.setAttribute("erase", static_cast<int>(_mapDrawPoint.erase()));
    element.setAttribute("smooth", static_cast<int>(_mapDrawPoint.smooth()));

    return element;
}

void UndoFowPoint::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapDrawPoint.setX(element.attribute(QString("x")).toInt());
    _mapDrawPoint.setY(element.attribute(QString("y")).toInt());
    _mapDrawPoint.setRadius(element.attribute(QString("radius")).toInt());
    _mapDrawPoint.setBrushType(element.attribute(QString("brushtype")).toInt());
    _mapDrawPoint.setErase(static_cast<bool>(element.attribute("erase",QString::number(1)).toInt()));
    _mapDrawPoint.setSmooth(static_cast<bool>(element.attribute("smooth",QString::number(1)).toInt()));
}

int UndoFowPoint::getType() const
{
    return DMHelper::ActionType_Point;
}

UndoFowBase* UndoFowPoint::clone() const
{
    return new UndoFowPoint(_layer, _mapDrawPoint);
}

const MapDrawPoint& UndoFowPoint::mapDrawPoint() const
{
    return _mapDrawPoint;
}

MapDrawPoint& UndoFowPoint::mapDrawPoint()
{
    return _mapDrawPoint;
}
