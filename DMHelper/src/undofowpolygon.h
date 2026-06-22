#ifndef UNDOFOWPOLYGON_H
#define UNDOFOWPOLYGON_H

#include "undofowbase.h"
#include "mapcontent.h"

class UndoFowPolygon : public UndoFowBase
{
public:
    UndoFowPolygon(LayerFow* layer, const MapEditPolygon& mapEditPolygon);

    virtual void apply() override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

    virtual const MapEditPolygon& mapEditPolygon() const;
    virtual MapEditPolygon& mapEditPolygon();

protected:
    MapEditPolygon _mapEditPolygon;
};

#endif // UNDOFOWPOLYGON_H
