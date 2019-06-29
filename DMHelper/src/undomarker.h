#ifndef UNDOMARKER_H
#define UNDOMARKER_H

#include "undobase.h"
#include "mapcontent.h"

class MapMarkerGraphicsItem;

class UndoMarker : public UndoBase
{
public:
    UndoMarker(Map& map, const MapMarker& marker);
    virtual ~UndoMarker();

    virtual void undo();
    virtual void redo();
    virtual void apply(bool preview, QPaintDevice* target) const;
    virtual void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;
    virtual void inputXML(const QDomElement &element, bool isImport);

    virtual int getType() const;
    virtual UndoBase* clone() const;

    const MapMarker& marker() const;

protected:
    MapMarker _marker;
    MapMarkerGraphicsItem* _markerGraphicsItem;

};

#endif // UNDOMARKER_H
