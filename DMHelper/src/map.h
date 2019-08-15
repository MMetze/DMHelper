#ifndef MAP_H
#define MAP_H

#include "adventureitem.h"
#include "mapcontent.h"
#include <QList>
#include <QImage>

class QDomDocument;
class QDomElement;
class QUndoStack;
class MapFrame;
class AudioTrack;

class Map : public AdventureItem
{
    Q_OBJECT
public:
    explicit Map(const QString& mapName, const QString& fileName, QObject *parent = nullptr);
    explicit Map(const QDomElement& element, bool isImport, QObject *parent = nullptr);
    explicit Map(const Map &obj);  // copy constructor

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;

    QString getName() const;
    void setName(const QString& newName);

    QString getFileName() const;
    void setFileName(const QString& newFileName);

    AudioTrack* getAudioTrack();
    QUuid getAudioTrackId();
    void setAudioTrack(AudioTrack* track);

    QUndoStack* getUndoStack() const;
    void applyPaintTo(QImage* target, QColor clearColor, int index);
    MapFrame* getRegisteredWindow() const;

    MapMarker* getMapMarker(int id);

    bool isInitialized();
    void setExternalFoWImage(QImage externalImage);
    QImage getBackgroundImage();
    QImage getFoWImage();

    void paintFoWPoint( QPoint point, const MapDraw& mapDraw, QPaintDevice* target, bool preview );
    void paintFoWRect( QRect rect, const MapEditShape& mapEditShape, QPaintDevice* target, bool preview );
    void fillFoW( QColor color, QPaintDevice* target );
    void undoPaint();
    QImage getBWFoWImage();
    QImage getBWFoWImage(const QImage &img);
    QImage getBWFoWImage(const QSize &size);
    QImage getPublishImage();
    QImage getPublishImage(const QRect& rect);
    QImage getShrunkPublishImage();

    QImage getPreviewImage();

signals:
    void dirty();
    void changed();

public slots:
    void registerWindow(MapFrame* mapFrame);
    void unregisterWindow(MapFrame* mapFrame);

    void initialize();
    void uninitialize();

private:

    QString _name;
    QString _filename;
    QUndoStack* _undoStack;
    MapFrame* _mapFrame;
    QList<MapMarker> _markerList;
    QUuid _audioTrackId;

    bool _initialized;
    QImage _imgBackground;
    QImage _imgFow;
};

#endif // MAP_H
