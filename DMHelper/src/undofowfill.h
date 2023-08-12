#ifndef UNDOFOWFILL_H
#define UNDOFOWFILL_H

#include "undofowbase.h"
#include "mapcontent.h"

class UndoFowFill : public UndoFowBase
{
public:
    UndoFowFill(LayerFow* layer, const MapEditFill& mapEditFill);

    virtual void apply() override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

    virtual const MapEditFill& mapEditFill() const;
    virtual MapEditFill& mapEditFill();

protected:
    MapEditFill _mapEditFill;

};

#endif // UNDOFOWFILL_H
