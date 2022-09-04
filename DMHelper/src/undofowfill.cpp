#include "undofowfill.h"
#include "layerfow.h"
#include "dmconstants.h"
#include <QDomElement>

UndoFowFill::UndoFowFill(LayerFow* layer, const MapEditFill& mapEditFill) :
    UndoFowBase(layer, QString("Fill")),
    _mapEditFill(mapEditFill)
{
}

void UndoFowFill::apply()
{
    if(_layer)
        _layer->fillFoW(_mapEditFill.color());

    /*
    if(_layer)
    {
        QColor applyColor = _mapEditFill.color();
        if(preview)
            applyColor.setAlpha(_mapEditFill.color().alpha() / 2);

        _layer->fillFoW(applyColor,target);
    }
    */
}

QDomElement UndoFowFill::outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    // TODO: remove color from Fill
    element.setAttribute( "r", _mapEditFill.color().red() );
    element.setAttribute( "g", _mapEditFill.color().green() );
    element.setAttribute( "b", _mapEditFill.color().blue() );
    element.setAttribute( "a", _mapEditFill.color().alpha() );

    return element;
}

void UndoFowFill::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _mapEditFill.setRed(element.attribute( QString("r")).toInt());
    _mapEditFill.setGreen(element.attribute( QString("g")).toInt());
    _mapEditFill.setBlue(element.attribute( QString("b")).toInt());
    _mapEditFill.setAlpha(element.attribute( QString("a")).toInt());
}

int UndoFowFill::getType() const
{
    return DMHelper::ActionType_Fill;
}

UndoFowBase* UndoFowFill::clone() const
{
    return new UndoFowFill(_layer, _mapEditFill);
}

const MapEditFill& UndoFowFill::mapEditFill() const
{
    return _mapEditFill;
}

MapEditFill& UndoFowFill::mapEditFill()
{
    return _mapEditFill;
}
