#include "undofowpath.h"
#include "layerfow.h"
#include "dmconstants.h"
#include <QDomElement>

UndoFowPath::UndoFowPath(LayerFow* layer, const MapDrawPath& mapDrawPath) :
    UndoFowBase(layer, QString("Paint Path")),
    _mapDrawPath(mapDrawPath)
{
}

void UndoFowPath::apply()
{
    if(_layer)
    {
        for(int i = 0; i < _mapDrawPath.points().count(); ++i)
        {
            _layer->paintFoWPoint(_mapDrawPath.points().at(i), _mapDrawPath);
        }
    }
}

QDomElement UndoFowPath::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("radius", _mapDrawPath.radius());
    element.setAttribute("brushtype", _mapDrawPath.brushType());
    element.setAttribute("erase", static_cast<int>(_mapDrawPath.erase()));
    element.setAttribute("smooth", static_cast<int>(_mapDrawPath.smooth()));
    QDomElement pointsElement = doc.createElement("points");
    for(int i = 0; i < _mapDrawPath.points().count(); ++i)
    {
        QDomElement pointElement = doc.createElement("point");
        pointElement.setAttribute("x", _mapDrawPath.points().at(i).x());
        pointElement.setAttribute("y", _mapDrawPath.points().at(i).y());
        pointsElement.appendChild(pointElement);
    }
    element.appendChild(pointsElement);

    return element;
}

void UndoFowPath::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapDrawPath.setRadius(element.attribute( QString("radius")).toInt());
    _mapDrawPath.setBrushType(element.attribute( QString("brushtype")).toInt());
    _mapDrawPath.setErase(static_cast<bool>(element.attribute("erase",QString::number(1)).toInt()));
    _mapDrawPath.setSmooth(static_cast<bool>(element.attribute("smooth",QString::number(1)).toInt()));

    QDomElement pointsElement = element.firstChildElement(QString("points"));
    if(!pointsElement.isNull())
    {
        QDomElement pointElement = pointsElement.firstChildElement(QString("point"));
        while(!pointElement.isNull())
        {
            QPoint newPoint;
            newPoint.setX(pointElement.attribute(QString("x")).toInt());
            newPoint.setY(pointElement.attribute(QString("y")).toInt());
            addPoint(newPoint);
            pointElement = pointElement.nextSiblingElement(QString("point"));
        }
    }
}

int UndoFowPath::getType() const
{
    return DMHelper::ActionType_Path;
}

UndoFowBase* UndoFowPath::clone() const
{
    return new UndoFowPath(_layer, _mapDrawPath);
}

void UndoFowPath::addPoint(QPoint aPoint)
{
    _mapDrawPath.addPoint(aPoint);
    if(_layer)
        _layer->paintFoWPoint(aPoint, _mapDrawPath);

    // TODO?
    /*
    if(_layer)
    {
        _mapDrawPath.addPoint(aPoint);
        _layer->paintFoWPoint(aPoint, _mapDrawPath);
        // TODO?
        //_map->updateFoW();
    }
    */
}

const MapDrawPath& UndoFowPath::mapDrawPath() const
{
    return _mapDrawPath;
}

MapDrawPath& UndoFowPath::mapDrawPath()
{
    return _mapDrawPath;
}
