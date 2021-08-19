#ifndef MAPFRAMESCENE_H
#define MAPFRAMESCENE_H

#include <QGraphicsScene>

class MapFrameScene : public QGraphicsScene
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

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    virtual void wheelEvent(QGraphicsSceneWheelEvent* wheelEvent);
    virtual void keyPressEvent(QKeyEvent* keyEvent);
    virtual void keyReleaseEvent(QKeyEvent* keyEvent);

    bool isMapMovement(QGraphicsSceneMouseEvent* mouseEvent);

    bool _spaceDown;
};

#endif // MAPFRAMESCENE_H
