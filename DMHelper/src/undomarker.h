#ifndef UNDOMARKER_H
#define UNDOMARKER_H

#include "undofowbase.h"
#include "mapcontent.h"

class MapMarkerGraphicsItem;

// TODO: Layers

class UndoMarker : public UndoFowBase
{
public:
    UndoMarker(LayerFow* layer, const MapMarker& marker);
    virtual ~UndoMarker() override;

    virtual void undo() override;
    virtual void redo() override;
    virtual void apply() override;
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual void setRemoved(bool removed) override;

    virtual int getType() const override;
    virtual UndoFowBase* clone() const override;

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
