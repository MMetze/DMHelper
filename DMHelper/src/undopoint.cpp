#include "undopoint.h"
#include "mapframe.h"
#include "map.h"
#include "dmconstants.h"
#include <QDomElement>

UndoPoint::UndoPoint(Map& map, const MapDrawPoint& mapDrawPoint) :
    UndoBase(map, QString("Paint")),
    _mapDrawPoint(mapDrawPoint)
{
}

void UndoPoint::undo()
{
    if(_map.getRegisteredWindow())
        _map.getRegisteredWindow()->undoPaint();
}

void UndoPoint::redo()
{
    if(_map.getRegisteredWindow())
    {
        apply(true, 0);
        _map.getRegisteredWindow()->updateFoW();
    }
}

void UndoPoint::apply( bool preview, QPaintDevice* target ) const
{
    _map.paintFoWPoint( _mapDrawPoint.point(), _mapDrawPoint, target, preview );
}

void UndoPoint::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);

    element.setAttribute( "x", _mapDrawPoint.point().x() );
    element.setAttribute( "y", _mapDrawPoint.point().y() );
    element.setAttribute( "radius", _mapDrawPoint.radius() );
    element.setAttribute( "brushtype", _mapDrawPoint.brushType() );
    element.setAttribute( "erase", (int)_mapDrawPoint.erase() );
}

void UndoPoint::inputXML(const QDomElement &element)
{
    _mapDrawPoint.setX(element.attribute( QString("x") ).toInt());
    _mapDrawPoint.setY(element.attribute( QString("y") ).toInt());
    _mapDrawPoint.setRadius(element.attribute( QString("radius") ).toInt());
    _mapDrawPoint.setBrushType(element.attribute( QString("brushtype") ).toInt());
    _mapDrawPoint.setErase((bool)(element.attribute("erase",QString::number(1)).toInt()));
}

int UndoPoint::getType() const
{
    return DMHelper::ActionType_Point;
}

const MapDrawPoint& UndoPoint::mapDrawPoint() const
{
    return _mapDrawPoint;
}

MapDrawPoint& UndoPoint::mapDrawPoint()
{
    return _mapDrawPoint;
}
