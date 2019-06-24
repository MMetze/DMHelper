#ifndef MAPFRAME_H
#define MAPFRAME_H

#include <QWidget>
#include <QGraphicsScene>
#include <QFileInfo>
#include <QImage>
#include <QRubberBand>
#include "undopath.h"

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
    explicit MapFrame(QWidget *parent = 0);
    ~MapFrame();

    void setMap(Map* map);

    MapMarkerGraphicsItem* addMapMarker(MapMarker& marker);
    void mapMarkerMoved(int markerId);
    void editMapMarker(int markerId);

    virtual bool eventFilter(QObject *obj, QEvent *event);

    QAction* getUndoAction(QObject* parent);
    QAction* getRedoAction(QObject* parent);

signals:
    void publishImage(QImage img, QColor color);
    void openPreview();
    void windowClosed(MapFrame* mapFrame);
    void dirty();
    void startTrack(AudioTrack* track);

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

protected:
    void initializeFoW();
    void uninitializeFoW();
    void loadTracks();

    virtual void hideEvent(QHideEvent * event);

    bool execEventFilterSelectZoom(QObject *obj, QEvent *event);
    bool execEventFilterEditModeFoW(QObject *obj, QEvent *event);
    bool execEventFilterEditModeEdit(QObject *obj, QEvent *event);
    bool execEventFilterEditModeMove(QObject *obj, QEvent *event);

protected slots:
    void setMapCursor();
    void publishModeVisibleClicked();
    void publishModeZoomClicked();
    void trackSelected(int index);
    void setScale(qreal s);

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

    Map* _mapSource;
};

#endif // MAPFRAME_H
