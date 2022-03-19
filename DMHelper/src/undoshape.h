#ifndef UNDOSHAPE_H
#define UNDOSHAPE_H

#include "undobase.h"
#include "mapcontent.h"

class UndoShape : public UndoBase
{
public:
    UndoShape(Map* map, const MapEditShape& mapEditShape);

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply( bool preview, QPaintDevice* target ) const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoBase* clone() const override;

    virtual const MapEditShape& mapEditShape() const;
    virtual MapEditShape& mapEditShape();

protected:
    MapEditShape _mapEditShape;
};

#endif // UNDOSHAPE_H
