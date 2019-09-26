#ifndef MAPFRAME_H
#define MAPFRAME_H

#include <QWidget>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QImage>
#include <QRubberBand>
#include <QMutex>
#include "undopath.h"

//#define ANIMATED_MAPS


#ifdef ANIMATED_MAPS
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
    #include <vlc/vlc.h>
#endif

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

#ifdef ANIMATED_MAPS
    void* lockCallback(void **planes);
    void unlockCallback(void *picture, void *const *planes);
    void displayCallback(void *picture);
    unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    void cleanupCallback();
    void exitEventCallback();
#endif


signals:
    void publishImage(QImage img, QColor color);
    void openPreview();
    void windowClosed(MapFrame* mapFrame);
    void dirty();
    void startTrack(AudioTrack* track);
    void showPublishWindow();

#ifdef ANIMATED_MAPS
    void animationStarted(QColor color);
    void animateImage(QImage img);
#endif

public slots:
    void updateFoW();
    void resetFoW();
    void clearFoW();
    void undoPaint();
    void publishFoWImage();
    void clear();

    void editModeToggled(int editMode);

    void zoomIn();
    void zoomOut();
    void zoomOne();
    void zoomFit();
    void zoomSelect();
    void cancelSelect();

#ifdef ANIMATED_MAPS
    void targetResized(const QSize& newSize);
#endif

protected:
    void initializeFoW();
    void uninitializeFoW();
    void loadTracks();

    virtual void hideEvent(QHideEvent * event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    virtual void timerEvent(QTimerEvent *event) override;

    bool execEventFilterSelectZoom(QObject *obj, QEvent *event);
    bool execEventFilterEditModeFoW(QObject *obj, QEvent *event);
    bool execEventFilterEditModeEdit(QObject *obj, QEvent *event);
    bool execEventFilterEditModeMove(QObject *obj, QEvent *event);

    void cleanupBuffers();

protected slots:
    void setMapCursor();
    void publishModeVisibleClicked();
    void publishModeZoomClicked();
    void trackSelected(int index);
    void setScale(qreal s);

    //void rotateCCW();
    //void rotateCW();

#ifdef ANIMATED_MAPS
    void executeAnimateImage();
#endif

private:
    Ui::MapFrame *ui;

    QGraphicsScene* _scene;
    QGraphicsPixmapItem* _background;
    QGraphicsPixmapItem* _fow;

    bool _mouseDown;
    QPoint _mouseDownPos;
    UndoPath* _undoPath;

    QRubberBand* _rubberBand;
    qreal _scale;
    //int _rotation;

    Map* _mapSource;

#ifdef ANIMATED_MAPS
    libvlc_instance_t *vlcInstance;
    libvlc_media_list_player_t *vlcListPlayer;
    unsigned int _nativeWidth;
    unsigned int _nativeHeight;
    uchar* _nativeBufferNotAligned;
    uchar* _nativeBuffer;
    QMutex* _mutex;
    QImage _loadImage;
    bool _newImage;
    int _timerId;
    QTimer* _publishTimer;
    QImage _bwFoWImage;
    QSize _targetSize;
#endif

};

#endif // MAPFRAME_H
