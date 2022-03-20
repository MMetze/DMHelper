#include "undofill.h"
#include "mapframe.h"
#include "map.h"
#include "dmconstants.h"
#include <QDomElement>

UndoFill::UndoFill(Map* map, const MapEditFill& mapEditFill) :
    UndoBase(map, QString("Fill")),
    _mapEditFill(mapEditFill)
{
}

void UndoFill::undo()
{
    if(_map)
        _map->undoPaint();
}

void UndoFill::redo()
{
    if(_map)
    {
        apply(true, nullptr);
        _map->updateFoW();
    }
}

void UndoFill::apply(bool preview, QPaintDevice* target) const
{
    if(_map)
    {
        QColor applyColor = _mapEditFill.color();
        if(preview)
            applyColor.setAlpha(_mapEditFill.color().alpha() / 2);

        _map->fillFoW(applyColor,target);
    }
}

QDomElement UndoFill::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute( "r", _mapEditFill.color().red() );
    element.setAttribute( "g", _mapEditFill.color().green() );
    element.setAttribute( "b", _mapEditFill.color().blue() );
    element.setAttribute( "a", _mapEditFill.color().alpha() );

    return element;
}

void UndoFill::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapEditFill.setRed(element.attribute( QString("r")).toInt());
    _mapEditFill.setGreen(element.attribute( QString("g")).toInt());
    _mapEditFill.setBlue(element.attribute( QString("b")).toInt());
    _mapEditFill.setAlpha(element.attribute( QString("a")).toInt());
}

int UndoFill::getType() const
{
    return DMHelper::ActionType_Fill;
}

UndoBase* UndoFill::clone() const
{
    return new UndoFill(_map, _mapEditFill);
}

const MapEditFill& UndoFill::mapEditFill() const
{
    return _mapEditFill;
}

MapEditFill& UndoFill::mapEditFill()
{
    return _mapEditFill;
}
