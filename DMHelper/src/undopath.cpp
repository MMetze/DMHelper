#include "undopath.h"
#include "mapframe.h"
#include "map.h"
#include "dmconstants.h"
#include <QDomElement>

UndoPath::UndoPath(Map& map, const MapDrawPath& mapDrawPath) :
    UndoBase(map, QString("Paint Path")),
    _mapDrawPath(mapDrawPath)
{
}

void UndoPath::undo()
{
    if( _map.getRegisteredWindow() )
        _map.getRegisteredWindow()->undoPaint();
}

void UndoPath::redo()
{
    if( _map.getRegisteredWindow() )
    {
        apply(true, nullptr);
        _map.getRegisteredWindow()->updateFoW();
    }
}

void UndoPath::apply( bool preview, QPaintDevice* target ) const
{
    for( int i = 0; i < _mapDrawPath.points().count(); ++i )
    {
        _map.paintFoWPoint( _mapDrawPath.points().at(i), _mapDrawPath, target, preview );
    }
}

void UndoPath::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute( "radius", _mapDrawPath.radius() );
    element.setAttribute( "brushtype", _mapDrawPath.brushType() );
    element.setAttribute( "erase", static_cast<int>(_mapDrawPath.erase()) );
    QDomElement pointsElement = doc.createElement( "points" );
    for( int i = 0; i < _mapDrawPath.points().count(); ++i )
    {
        QDomElement pointElement = doc.createElement( "point" );
        pointElement.setAttribute( "x", _mapDrawPath.points().at(i).x() );
        pointElement.setAttribute( "y", _mapDrawPath.points().at(i).y() );
        pointsElement.appendChild(pointElement);
    }
    element.appendChild(pointsElement);
}

void UndoPath::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapDrawPath.setRadius(element.attribute( QString("radius") ).toInt());
    _mapDrawPath.setBrushType(element.attribute( QString("brushtype") ).toInt());
    _mapDrawPath.setErase(static_cast<bool>(element.attribute("erase",QString::number(1)).toInt()));

    QDomElement pointsElement = element.firstChildElement( QString("points") );
    if( !pointsElement.isNull() )
    {
        QDomElement pointElement = pointsElement.firstChildElement( QString("point") );
        while( !pointElement.isNull() )
        {
            QPoint newPoint;
            newPoint.setX(pointElement.attribute( QString("x") ).toInt());
            newPoint.setY(pointElement.attribute( QString("y") ).toInt());
            addPoint(newPoint);
            pointElement = pointElement.nextSiblingElement( QString("point") );
        }
    }
}

int UndoPath::getType() const
{
    return DMHelper::ActionType_Path;
}

UndoBase* UndoPath::clone() const
{
    return new UndoPath(_map, _mapDrawPath);
}

void UndoPath::addPoint(QPoint aPoint)
{
    _mapDrawPath.addPoint(aPoint);
    if( _map.getRegisteredWindow() )
    {
        _map.paintFoWPoint( aPoint, _mapDrawPath, nullptr, true );
        _map.getRegisteredWindow()->updateFoW();
    }
}

const MapDrawPath& UndoPath::mapDrawPath() const
{
    return _mapDrawPath;
}

MapDrawPath& UndoPath::mapDrawPath()
{
    return _mapDrawPath;
}
