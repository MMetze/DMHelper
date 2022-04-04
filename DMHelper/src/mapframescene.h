#ifndef MAPFRAMESCENE_H
#define MAPFRAMESCENE_H

#include "camerascene.h"

class MapFrameScene : public CameraScene
{
    Q_OBJECT
public:
    MapFrameScene(QObject* parent = nullptr);

signals:
    void mapMousePress(const QPointF& pos);
    void mapMouseMove(const QPointF& pos);
    void mapMouseRelease(const QPointF& pos);
    void mapZoom(int delta);

    void addMarker(const QPointF& pos);
    void editMarker(int marker);
    void deleteMarker(int marker);
    void centerView(const QPointF& pos);
    void clearFoW();

protected slots:
    void handleAddMarker();
    void handleEditMarker();
    void handleDeleteMarker();
    void handleCenterView();
    void handleClearFoW();

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void wheelEvent(QGraphicsSceneWheelEvent* wheelEvent);
    virtual void keyPressEvent(QKeyEvent* keyEvent);
    virtual void keyReleaseEvent(QKeyEvent* keyEvent);

    bool isMapMovement(QGraphicsSceneMouseEvent* mouseEvent);

    bool _spaceDown;
    QGraphicsItem* _contextMenuItem;
    QPointF _contextMenuPos;

};

#endif // MAPFRAMESCENE_H
