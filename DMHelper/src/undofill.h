#ifndef UNDOFILL_H
#define UNDOFILL_H

#include "undobase.h"
#include "mapcontent.h"

class UndoFill : public UndoBase
{
public:
    UndoFill(Map& map, const MapEditFill& mapEditFill);

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply( bool preview, QPaintDevice* target ) const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoBase* clone() const override;

    virtual const MapEditFill& mapEditFill() const;
    virtual MapEditFill& mapEditFill();

protected:
    MapEditFill _mapEditFill;

};

#endif // UNDOFILL_H
