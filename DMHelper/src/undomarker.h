#ifndef UNDOMARKER_H
#define UNDOMARKER_H

#include <QObject>
#include "undofowbase.h"
#include "mapcontent.h"

class MapMarkerGraphicsItem;
class QGraphicsScene;

//class UndoMarker : public UndoFowBase
class UndoMarker : public QObject
{
    Q_OBJECT
public:
    //UndoMarker(LayerFow* layer, const MapMarker& marker);
    UndoMarker(const MapMarker& marker);
    virtual ~UndoMarker() override;

//    virtual void undo() override;
//    virtual void redo() override;
//    virtual void apply();
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;
    virtual void inputXML(const QDomElement &element, bool isImport);

//    virtual bool isRemoved() const;
//    virtual void setRemoved(bool removed);

//    virtual int getType() const;
    virtual UndoMarker* clone() const;

    void setMarker(const MapMarker& marker);
    const MapMarker& getMarker() const;
    MapMarker& getMarker();

    void createMarkerItem(QGraphicsScene* scene, qreal initialScale);
    void cleanupMarkerItem();
    MapMarkerGraphicsItem* getMarkerItem() const;
    //void setMarkerItem(MapMarkerGraphicsItem* markerItem);

public slots:
    void moveMapMarker();
    void editMapMarker();
    void unsetPartySelected();
    void activateMapMarker();

signals:
    void mapMarkerMoved(UndoMarker* marker);
    void mapMarkerEdited(UndoMarker* marker);
    void unselectParty(bool unselect);
    void mapMarkerActivated(UndoMarker* marker);

protected:
    MapMarker _marker;
    MapMarkerGraphicsItem* _markerGraphicsItem;
    //bool _removed;
};

#endif // UNDOMARKER_H
