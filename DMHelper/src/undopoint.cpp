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
        apply(true, nullptr);
        _map.getRegisteredWindow()->updateFoW();
    }
}

void UndoPoint::apply( bool preview, QPaintDevice* target ) const
{
    _map.paintFoWPoint( _mapDrawPoint.point(), _mapDrawPoint, target, preview );
}

void UndoPoint::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute( "x", _mapDrawPoint.point().x() );
    element.setAttribute( "y", _mapDrawPoint.point().y() );
    element.setAttribute( "radius", _mapDrawPoint.radius() );
    element.setAttribute( "brushtype", _mapDrawPoint.brushType() );
    element.setAttribute( "erase", static_cast<int>(_mapDrawPoint.erase()) );
}

void UndoPoint::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapDrawPoint.setX(element.attribute( QString("x") ).toInt());
    _mapDrawPoint.setY(element.attribute( QString("y") ).toInt());
    _mapDrawPoint.setRadius(element.attribute( QString("radius") ).toInt());
    _mapDrawPoint.setBrushType(element.attribute( QString("brushtype") ).toInt());
    _mapDrawPoint.setErase(static_cast<bool>(element.attribute("erase",QString::number(1)).toInt()));
}

int UndoPoint::getType() const
{
    return DMHelper::ActionType_Point;
}

UndoBase* UndoPoint::clone() const
{
    return new UndoPoint(_map, _mapDrawPoint);
}

const MapDrawPoint& UndoPoint::mapDrawPoint() const
{
    return _mapDrawPoint;
}

MapDrawPoint& UndoPoint::mapDrawPoint()
{
    return _mapDrawPoint;
}
