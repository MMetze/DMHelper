#include "undofowpolygon.h"
#include "layerfow.h"

UndoFowPolygon::UndoFowPolygon(LayerFow* layer, const MapEditPolygon& mapEditPolygon) :
    UndoFowBase(layer, QString("Paint")),
    _mapEditPolygon(mapEditPolygon)
{
}

void UndoFowPolygon::apply()
{
    if(_layer)
        _layer->paintFoWPolygon(_mapEditPolygon);
}

QDomElement UndoFowPolygon::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("erase", static_cast<int>(_mapEditPolygon.erase()));
    element.setAttribute("smooth", static_cast<int>(_mapEditPolygon.smooth()));

    QPolygon polygon = _mapEditPolygon.polygon();
    for(int i = 0; i < polygon.count(); ++i)
    {
        QDomElement pointElement = doc.createElement(QString("point"));
        pointElement.setAttribute("x", polygon.at(i).x());
        pointElement.setAttribute("y", polygon.at(i).y());
        element.appendChild(pointElement);
    }

    return element;
}

void UndoFowPolygon::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapEditPolygon.setErase(static_cast<bool>(element.attribute("erase", QString::number(1)).toInt()));
    _mapEditPolygon.setSmooth(static_cast<bool>(element.attribute("smooth", QString::number(0)).toInt()));

    QPolygon polygon;
    QDomElement pointElement = element.firstChildElement(QString("point"));
    while(!pointElement.isNull())
    {
        polygon.append(QPoint(pointElement.attribute("x").toInt(), pointElement.attribute("y").toInt()));
        pointElement = pointElement.nextSiblingElement(QString("point"));
    }
    _mapEditPolygon.setPolygon(polygon);
}

int UndoFowPolygon::getType() const
{
    return DMHelper::ActionType_Polygon;
}

UndoFowBase* UndoFowPolygon::clone() const
{
    return new UndoFowPolygon(_layer, _mapEditPolygon);
}

const MapEditPolygon& UndoFowPolygon::mapEditPolygon() const
{
    return _mapEditPolygon;
}

MapEditPolygon& UndoFowPolygon::mapEditPolygon()
{
    return _mapEditPolygon;
}
