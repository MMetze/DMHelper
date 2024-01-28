#ifndef MAP_H
#define MAP_H

#include "campaignobjectbase.h"
#include "mapcontent.h"
#include "mapcolorizefilter.h"
#include "layerscene.h"
#include <QList>
#include <QImage>
#include <QPixmap>

class QDomDocument;
class QDomElement;
class UndoFowBase;
class AudioTrack;
class Party;
class UndoMarker;

class Map : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Map(const QString& mapName = QString(), QObject *parent = nullptr);
    virtual ~Map() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getObjectType() const override;
    virtual QIcon getDefaultIcon() override;

    // Local
    QString getFileName() const;
    bool setFileName(const QString& newFileName);

    QColor getMapColor() const;
    void setMapColor(const QColor& color);
    QSize getMapSize() const;
    void setMapSize(QSize size);
    int getGridCount() const;
    void setGridCount(int gridCount);

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId() const;
    void setAudioTrack(AudioTrack* track);

    bool getPlayAudio() const;
    void setPlayAudio(bool playAudio);

    Party* getParty();
    QString getPartyAltIcon() const;
    QUuid getPartyId() const;
    bool getShowParty() const;
    const QPoint& getPartyIconPos() const;
    int getPartyScale() const;
    QPixmap getPartyPixmap();

    int getDistanceLineType() const;
    QColor getDistanceLineColor() const;
    int getDistanceLineWidth() const;
    int getMapScale() const;

    const QRect& getMapRect() const;
    void setMapRect(const QRect& mapRect);

    const QRect& getCameraRect() const;

    void initializeMarkers(QGraphicsScene* scene);
    void cleanupMarkers();
    bool getShowMarkers() const;
    QList<UndoMarker*> getMarkers();
    int getMarkerCount() const;

    void addMapItem(MapDraw* mapItem);
    void removeMapItem(MapDraw* mapItem);
    int getMapItemCount() const;
    MapDraw* getMapItem(int index);

    bool isInitialized();
    bool isValid();
    LayerScene& getLayerScene();
    const LayerScene& getLayerScene() const;
    // TODO - remove
    void setExternalFoWImage(QImage externalImage);
    QImage getUnfilteredBackgroundImage();
    QImage getBackgroundImage();
    QImage getFoWImage();
    bool isCleared();

    QImage getGrayImage();

    bool isFilterApplied() const;
    MapColorizeFilter getFilter() const;

    QImage getPreviewImage();

    void addMarker(UndoMarker* marker);
    void removeMarker(UndoMarker* marker);

signals:
    void partyChanged(Party* party);
    void partyIconChanged(const QString& partyIcon);
    void partyIconPosChanged(const QPoint& pos);
    void showPartyChanged(bool showParty);
    void partyScaleChanged(int partyScale);
    void mapScaleChanged(int mapScale);

    void distanceLineColorChanged(const QColor& color);
    void distanceLineTypeChanged(int lineType);
    void distanceLineWidthChanged(int lineWidth);

    void mapMarkerMoved(UndoMarker* marker);
    void mapMarkerEdited(UndoMarker* marker);
    void unselectParty(bool unselect);
    void mapMarkerActivated(UndoMarker* marker);

    void showMarkersChanged(bool showMarkers);

    void mapImageChanged(const QImage& image);

public slots:
    bool initialize(); // returns false only if reasonably believe this is a video file
    void uninitialize();

    void undoPaint();
    void updateFoW();

    void setParty(Party* party);
    void setPartyIcon(const QString& partyIcon);
    void setShowParty(bool showParty);
    void setPartyIconPos(const QPoint& pos);
    void setPartyScale(int partyScale);
    void setMapScale(int mapScale);

    void setDistanceLineColor(const QColor& color);
    void setDistanceLineType(int lineType);
    void setDistanceLineWidth(int lineWidth);

    void setShowMarkers(bool showMarkers);

    void setApplyFilter(bool applyFilter);
    void setFilter(const MapColorizeFilter& filter);

    void setCameraRect(const QRect& cameraRect);
    void setCameraRect(const QRectF& cameraRect);

protected slots:
    void initializePartyScale();

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    QUuid _audioTrackId;
    bool _playAudio;
    QRect _mapRect;
    QRect _cameraRect;

    bool _showPartyIcon;
    QUuid _partyId;
    QString _partyAltIcon;
    QPoint _partyIconPos;
    int _mapScale;
    int _gridCount;

    bool _showMarkers;
    QList<MapDraw*> _mapItems;

    bool _initialized;
    LayerScene _layerScene;
    QList<UndoFowBase*> _undoItems;
    int _lineType;
    QColor _lineColor;
    int _lineWidth;

    // For a generic map
    QColor _mapColor;
    QSize _mapSize;

    QList<UndoMarker*> _markerList;
};

#endif // MAP_H
