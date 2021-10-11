#ifndef UNDOMARKER_H
#define UNDOMARKER_H

#include "undobase.h"
#include "mapcontent.h"

class MapMarkerGraphicsItem;

class UndoMarker : public UndoBase
{
public:
    UndoMarker(Map& map, const MapMarker& marker);
    virtual ~UndoMarker() override;

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply(bool preview, QPaintDevice* target) const override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual void setRemoved(bool removed) override;

    virtual int getType() const override;
    virtual UndoBase* clone() const override;

    void setMarker(const MapMarker& marker);
    const MapMarker& getMarker() const;
    MapMarker& getMarker();

    MapMarkerGraphicsItem* getMarkerItem() const;
    void setMarkerItem(MapMarkerGraphicsItem* markerItem);

protected:
    MapMarker _marker;
    MapMarkerGraphicsItem* _markerGraphicsItem;

};

#endif // UNDOMARKER_H
