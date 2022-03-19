#ifndef UNDOPOINT_H
#define UNDOPOINT_H

#include "undobase.h"
#include "mapcontent.h"

class UndoPoint : public UndoBase
{
public:
    UndoPoint(Map* map, const MapDrawPoint& mapDrawPoint);

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply( bool preview, QPaintDevice* target ) const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoBase* clone() const override;

    virtual const MapDrawPoint& mapDrawPoint() const;
    virtual MapDrawPoint& mapDrawPoint();

protected:
    MapDrawPoint _mapDrawPoint;
};

#endif // UNDOPOINT_H
