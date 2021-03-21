#ifndef MAP_H
#define MAP_H

#include "campaignobjectbase.h"
#include "mapcontent.h"
#include <QList>
#include <QImage>

class QDomDocument;
class QDomElement;
class QUndoStack;
class AudioTrack;

class Map : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Map(const QString& mapName = QString(), const QString& fileName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;

    virtual QString getMD5() const override;
    virtual void setMD5(const QString& md5) override;

    virtual QString getFileName() const override;
    virtual void setFileName(const QString& newFileName) override;

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId();
    void setAudioTrack(AudioTrack* track);

    bool getPlayAudio() const;
    void setPlayAudio(bool playAudio);

    const QRect& getMapRect() const;
    void setMapRect(const QRect& mapRect);

    QUndoStack* getUndoStack() const;
    void applyPaintTo(QImage* target, QColor clearColor, int index, bool preview = false);

    MapMarker* getMapMarker(int id);

    bool isInitialized();
    void setExternalFoWImage(QImage externalImage);
    QImage getBackgroundImage();
    QImage getFoWImage();
    bool isCleared();

    void paintFoWPoint( QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview );
    void paintFoWRect( QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview );
    void fillFoW( QColor color, QPaintDevice* target );
    QImage getBWFoWImage();
    QImage getBWFoWImage(const QImage &img);
    QImage getBWFoWImage(const QSize &size);
    QImage getPublishImage();
    QImage getPublishImage(const QRect& rect);
    QImage getGrayImage();
    QImage getShrunkPublishImage();

    QImage getPreviewImage();

signals:
    void executeUndo();
    void requestFoWUpdate();

public slots:

    void initialize();
    void uninitialize();

    void undoPaint();
    void updateFoW();

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

    QString _filename;
    QUndoStack* _undoStack;
    QList<MapMarker> _markerList;
    QUuid _audioTrackId;
    bool _playAudio;
    QRect _mapRect;

    bool _initialized;
    QImage _imgBackground;
    QImage _imgFow;

    QString _md5;
};

#endif // MAP_H
