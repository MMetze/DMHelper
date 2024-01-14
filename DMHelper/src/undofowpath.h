#ifndef UNDOFOWPATH_H
#define UNDOFOWPATH_H

#include "undofowbase.h"
#include "mapcontent.h"

class UndoFowPath : public UndoFowBase
{
public:
    UndoFowPath(LayerFow* layer, const MapDrawPath& mapDrawPath);

    virtual void apply() override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

    void addPoint(QPoint aPoint);

    virtual const MapDrawPath& mapDrawPath() const;
    virtual MapDrawPath& mapDrawPath();

protected:
    MapDrawPath _mapDrawPath;

};

#endif // UNDOFOWPATH_H
