#ifndef UNDOPATH_H
#define UNDOPATH_H

#include "undobase.h"
#include "mapcontent.h"

class UndoPath : public UndoBase
{
public:
    UndoPath(Map& map, const MapDrawPath& mapDrawPath);

    virtual void undo();
    virtual void redo();
    virtual void apply( bool preview, QPaintDevice* target ) const;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const;
    virtual void inputXML(const QDomElement &element);

    virtual int getType() const;

    void addPoint(QPoint aPoint);

    virtual const MapDrawPath& mapDrawPath() const;
    virtual MapDrawPath& mapDrawPath();

protected:
    MapDrawPath _mapDrawPath;

};

#endif // UNDOPATH_H
