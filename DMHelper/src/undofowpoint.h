#ifndef UNDOFOWPOINT_H
#define UNDOFOWPOINT_H

#include "undofowbase.h"
#include "mapcontent.h"

class UndoFowPoint : public UndoFowBase
{
public:
    UndoFowPoint(LayerFow* layer, const MapDrawPoint& mapDrawPoint);

    virtual void apply() const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

    virtual const MapDrawPoint& mapDrawPoint() const;
    virtual MapDrawPoint& mapDrawPoint();

protected:
    MapDrawPoint _mapDrawPoint;
};

#endif // UNDOFOWPOINT_H
