#ifndef UNDOPOINT_H
#define UNDOPOINT_H

#include "undobase.h"
#include "mapcontent.h"

class UndoPoint : public UndoBase
{
public:
    UndoPoint(Map& map, const MapDrawPoint& mapDrawPoint);

    virtual void undo();
    virtual void redo();
    virtual void apply( bool preview, QPaintDevice* target ) const;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const;
    virtual void inputXML(const QDomElement &element);

    virtual int getType() const;

    virtual const MapDrawPoint& mapDrawPoint() const;
    virtual MapDrawPoint& mapDrawPoint();

protected:
    MapDrawPoint _mapDrawPoint;
};

#endif // UNDOPOINT_H
