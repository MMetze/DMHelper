#ifndef MAP_H
#define MAP_H

#include "campaignobjectbase.h"
#include "mapcontent.h"
#include "mapcolorizefilter.h"
#include <QList>
#include <QImage>
#include <QPixmap>

class QDomDocument;
class QDomElement;
class QUndoStack;
class AudioTrack;
class Party;
class UndoMarker;

class Map : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Map(const QString& mapName = QString(), const QString& fileName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;

    QString getFileName() const;
    void setFileName(const QString& newFileName);

    QColor getMapColor() const;
    void setMapColor(QColor color);
    QSize getMapSize() const;
    void setMapSize(QSize size);

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId();
    void setAudioTrack(AudioTrack* track);

    bool getPlayAudio() const;
    void setPlayAudio(bool playAudio);

    Party* getParty();
    QString getPartyAltIcon();
    QUuid getPartyId() const;
    bool getShowParty() const;
    const QPoint& getPartyIconPos() const;
    int getPartyScale() const;
    QPixmap getPartyPixmap();

    int getMapScale() const;

    const QRect& getMapRect() const;
    void setMapRect(const QRect& mapRect);

    QUndoStack* getUndoStack() const;
    void applyPaintTo(QImage* target, QColor clearColor, int index, bool preview = false);

    UndoMarker* getMapMarker(int id);
    bool getShowMarkers() const;

    bool isInitialized();
    void setExternalFoWImage(QImage externalImage);
    QImage getBackgroundImage();
    QImage getFoWImage();
    bool isCleared();

    void paintFoWPoint(QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview);
    void paintFoWRect(QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview);
    void fillFoW(QColor color, QPaintDevice* target);
    QImage getBWFoWImage();
    QImage getBWFoWImage(const QImage &img);
    QImage getBWFoWImage(const QSize &size);
    QImage getPublishImage();
    QImage getPublishImage(const QRect& rect);
    QImage getGrayImage();
    QImage getShrunkPublishImage(QRect* targetRect = nullptr);

    bool isFilterApplied() const;
    MapColorizeFilter getFilter() const;

    QImage getPreviewImage();

signals:
    void executeUndo();
    void requestFoWUpdate();
    void requestMapMarker(UndoMarker* undoEntry, MapMarker* marker);

    void partyChanged(Party* party);
    void partyIconChanged(const QString& partyIcon);
    void showPartyChanged(bool showParty);
    void partyScaleChanged(int partyScale);
    void mapScaleChanged(int mapScale);

    void showMarkersChanged(bool showMarkers);

public slots:
    void initialize();
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

    void setShowMarkers(bool showMarkers);

    void setApplyFilter(bool applyFilter);
    void setFilter(const MapColorizeFilter& filter);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    QString _filename;
    QUndoStack* _undoStack;
    QUuid _audioTrackId;
    bool _playAudio;
    QRect _mapRect;

    bool _showPartyIcon;
    QUuid _partyId;
    QString _partyAltIcon;
    QPoint _partyIconPos;
    int _partyScale;
    int _mapScale;

    bool _showMarkers;

    bool _initialized;
    QImage _imgBackground;
    QImage _imgFow;
    bool _filterApplied;
    MapColorizeFilter _filter;

    // For a generic map
    QColor _mapColor;
    QSize _mapSize;
};

#endif // MAP_H
