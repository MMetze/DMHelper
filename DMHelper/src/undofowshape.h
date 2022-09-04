#ifndef UNDOFOWSHAPE_H
#define UNDOFOWSHAPE_H

#include "undofowbase.h"
#include "mapcontent.h"

class UndoFowShape : public UndoFowBase
{
public:
    UndoFowShape(LayerFow* layer, const MapEditShape& mapEditShape);

    virtual void apply() override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

    virtual const MapEditShape& mapEditShape() const;
    virtual MapEditShape& mapEditShape();

protected:
    MapEditShape _mapEditShape;
};

#endif // UNDOFOWSHAPE_H
