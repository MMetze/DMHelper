#ifndef MAP_H
#define MAP_H

#include "campaignobjectbase.h"
#include "mapcontent.h"
#include "layerscene.h"
#include "layerimage.h"
#include "layerfow.h"
#include <QList>
#include <QImage>
#include <QPixmap>

class QDomDocument;
class QDomElement;
class UndoFowBase;
class AudioTrack;
class Party;
class UndoMarker;

class Map : public CampaignObjectBase//, public ILayerImageSource
{
    Q_OBJECT
public:
    explicit Map(const QString& mapName = QString(), QObject *parent = nullptr);
    virtual ~Map() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getObjectType() const override;

    // From ILayerImageSource
    //virtual const QImage& getImage() const override;

    // Local
    QString getFileName() const;
    bool setFileName(const QString& newFileName);

    QColor getMapColor() const;
    void setMapColor(const QColor& color);
    QSize getMapSize() const;
    void setMapSize(QSize size);

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

    UndoMarker* getMapMarker(int id);
    bool getShowMarkers() const;
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

    /*
    void paintFoWPoint(QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview);
    void paintFoWRect(QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview);
    void fillFoW(const QColor& color, QPaintDevice* target);
    QImage getBWFoWImage();
    QImage getBWFoWImage(const QImage &img);
    QImage getBWFoWImage(const QSize &size);
    */
//    QImage getPublishImage();
//    QImage getPublishImage(const QRect& rect);
    QImage getGrayImage();
//    QImage getShrunkPublishImage(QRect* targetRect = nullptr);
    QRect getShrunkPublishRect();

    bool isFilterApplied() const;
    MapColorizeFilter getFilter() const;

    QImage getPreviewImage();

signals:
    //void executeUndo();
    //void requestFoWUpdate();
    void requestMapMarker(UndoMarker* undoEntry, MapMarker* marker);

    void partyChanged(Party* party);
    void partyIconChanged(const QString& partyIcon);
    void partyIconPosChanged(const QPoint& pos);
    void showPartyChanged(bool showParty);
    void partyScaleChanged(int partyScale);
    void mapScaleChanged(int mapScale);

    void distanceLineColorChanged(const QColor& color);
    void distanceLineTypeChanged(int lineType);
    void distanceLineWidthChanged(int lineWidth);

    void showMarkersChanged(bool showMarkers);

    void mapImageChanged(const QImage& image);

public slots:
    bool initialize(); // returns false only if reasonably believe this is a video file
    void uninitialize();

    void undoPaint();
    void updateFoW();

    void addMapMarker(UndoMarker* undoEntry, MapMarker* marker);

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

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    QString _filename; // for compatibility only
    //QUndoStack* _undoStack;
    QUuid _audioTrackId;
    bool _playAudio;
    QRect _mapRect;
    QRect _cameraRect;

    bool _showPartyIcon;
    QUuid _partyId;
    QString _partyAltIcon;
    QPoint _partyIconPos;
    //int _partyScale;
    int _mapScale;

    bool _showMarkers;
    QList<MapDraw*> _mapItems;

    bool _initialized;
    LayerScene _layerScene;
    //QImage _imgBackground;
    //QImage _imgFow;
    QList<UndoFowBase*> _undoItems;
    //QImage _imgBWFow;
    //int _indexBWFow;
    //bool _filterApplied;
    //MapColorizeFilter _filter;
    int _lineType;
    QColor _lineColor;
    int _lineWidth;

    // For a generic map
    QColor _mapColor;
    QSize _mapSize;
};

#endif // MAP_H
