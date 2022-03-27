#ifndef UNDOPATH_H
#define UNDOPATH_H

#include "undobase.h"
#include "mapcontent.h"

class UndoPath : public UndoBase
{
public:
    UndoPath(Map* map, const MapDrawPath& mapDrawPath);

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply( bool preview, QPaintDevice* target ) const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual UndoBase* clone() const override;

    void addPoint(QPoint aPoint);

    virtual const MapDrawPath& mapDrawPath() const;
    virtual MapDrawPath& mapDrawPath();

protected:
    MapDrawPath _mapDrawPath;

};

#endif // UNDOPATH_H
