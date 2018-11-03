#ifndef UNDOFILL_H
#define UNDOFILL_H

#include "undobase.h"
#include "mapcontent.h"

class UndoFill : public UndoBase
{
public:
    UndoFill(Map& map, const MapEditFill& mapEditFill);

    virtual void undo();
    virtual void redo();
    virtual void apply( bool preview, QPaintDevice* target ) const;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const;
    virtual void inputXML(const QDomElement &element);

    virtual int getType() const;

    virtual const MapEditFill& mapEditFill() const;
    virtual MapEditFill& mapEditFill();

protected:
    MapEditFill _mapEditFill;

};

#endif // UNDOFILL_H
