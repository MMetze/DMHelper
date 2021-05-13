#ifndef MAPFRAME_H
#define MAPFRAME_H

#include "campaignobjectframe.h"
#include <QGraphicsScene>
#include <QFileInfo>
#include <QImage>
#include <QRubberBand>
#include "undopath.h"
#include "videoplayer.h"

namespace Ui {
class MapFrame;
}

class Map;
class MapMarkerGraphicsItem;
class AudioTrack;

class MapFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit MapFrame(QWidget *parent = nullptr);
    virtual ~MapFrame() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setMap(Map* map);

    MapMarkerGraphicsItem* addMapMarker(MapMarker& marker);
    void mapMarkerMoved(int markerId);
    void editMapMarker(int markerId);

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    QAction* getUndoAction(QObject* parent);
    QAction* getRedoAction(QObject* parent);

signals:
    void publishImage(QImage image);
    void publishMap(CampaignObjectBase* map);
    void openPreview();
    void windowClosed(MapFrame* mapFrame);
    void dirty();
    void startTrack(AudioTrack* track);
    void showPublishWindow();

    void animationStarted(CampaignObjectBase* animatedObject);
    void animateImage(QImage img);

    void zoomSelectChanged(bool enabled);
    void brushModeSet(int brushMode);

    void publishCancelled();
    //void publishCheckable(bool checkable);

public slots:
    void updateFoW();
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void undoPaint();
//    void publishFoWImage(bool publishing = false);
    void clear();

    void cancelPublish();

    void editModeToggled(int editMode);
    void setBrushMode(int brushMode);
    void brushSizeChanged(int size);

    void editMapFile();
    void zoomIn();
    void zoomOut();
    void zoomOne();
    void zoomFit();
    void zoomSelect(bool enabled);
    void cancelSelect();

    void setErase(bool enabled);
    void setSmooth(bool enabled);

    void setPublishZoom(bool enabled);
    void setPublishVisible(bool enabled);

    void targetResized(const QSize& newSize);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    //virtual void setBackgroundColor(QColor color) override;
    //void setRotation(int rotation);
    //void setColor(QColor color);

protected:
    void initializeFoW();
    void uninitializeFoW();
    void loadTracks();

    virtual void hideEvent(QHideEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

    bool execEventFilterSelectZoom(QObject *obj, QEvent *event);
    bool execEventFilterEditModeFoW(QObject *obj, QEvent *event);
    bool execEventFilterEditModeEdit(QObject *obj, QEvent *event);
    bool execEventFilterEditModeMove(QObject *obj, QEvent *event);

    void startPublishTimer();
    void stopPublishTimer();

    void createVideoPlayer(bool dmPlayer);
    void cleanupBuffers();

    void startAudioTrack();

protected slots:
    void setMapCursor();
    void drawEditCursor();
//    void publishModeVisibleClicked();
//    void publishModeZoomClicked();
    void rotatePublish();
    void trackSelected(int index);
    void setScale(qreal s);
    void storeViewRect();
    void loadViewRect();
    void resetPublishFoW();
    void audioPlaybackChecked();

private:
    Ui::MapFrame *ui;

    QGraphicsScene* _scene;
    QGraphicsPixmapItem* _backgroundImage;
    QGraphicsPixmapItem* _backgroundVideo;
    QGraphicsPixmapItem* _fow;

    bool _erase;
    bool _smooth;
    int _brushMode;
    int _brushSize;
    bool _publishZoom;
    bool _publishVisible;
    bool _isPublishing;
    bool _isVideo;

    int _rotation;
    //QColor _color;

    bool _mouseDown;
    QPoint _mouseDownPos;
    UndoPath* _undoPath;

    bool _zoomSelect;
    QRubberBand* _rubberBand;
    qreal _scale;

    Map* _mapSource;

    int _timerId;
    VideoPlayer* _videoPlayer;
    QSize _targetSize;
    QImage _bwFoWImage;

};

#endif // MAPFRAME_H
