#ifndef MAPFRAME_H
#define MAPFRAME_H

#include <QWidget>
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

class MapFrame : public QWidget
{
    Q_OBJECT

public:
    explicit MapFrame(QWidget *parent = nullptr);
    virtual ~MapFrame() override;

    void setMap(Map* map);

    MapMarkerGraphicsItem* addMapMarker(MapMarker& marker);
    void mapMarkerMoved(int markerId);
    void editMapMarker(int markerId);

    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    QAction* getUndoAction(QObject* parent);
    QAction* getRedoAction(QObject* parent);

signals:
    void publishImage(QImage img, QColor color);
    void openPreview();
    void windowClosed(MapFrame* mapFrame);
    void dirty();
    void startTrack(AudioTrack* track);
    void showPublishWindow();

    void animationStarted(QColor color);
    void animateImage(QImage img);

    void zoomSelectChanged(bool enabled);
    void brushModeSet(int brushMode);

public slots:
    void updateFoW();
    void fillFoW();
    void resetFoW();
    void clearFoW();
    void undoPaint();
    void publishFoWImage();
    void clear();

    void cancelPublish();

    void editModeToggled(int editMode);
    void setBrushMode(int brushMode);
    void brushSizeChanged(int size);

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
