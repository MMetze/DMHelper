#include "mapframescene.h"
#include "mapmarkergraphicsitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsItem>
#include <QKeyEvent>

MapFrameScene::MapFrameScene(QObject* parent) :
    QGraphicsScene(parent),
    _spaceDown(false)
{
}

void MapFrameScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(isMapMovement(mouseEvent))
    {
        emit mapMouseMove(mouseEvent->scenePos());
        mouseEvent->accept();
        return;
    }

    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void MapFrameScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(mouseEvent)
    {
        QList<QGraphicsItem*> mouseItems = items(mouseEvent->scenePos());
        int itemType = (mouseItems.count() > 0) ? mouseItems.at(0)->type() : -1;

        if((itemType != MapMarkerGraphicsItem::Type) && (isMapMovement(mouseEvent)))
        {
            emit mapMousePress(mouseEvent->scenePos());
            mouseEvent->accept();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(mouseEvent);
}

void MapFrameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{    
    if(mouseEvent)
    {
        QList<QGraphicsItem*> mouseItems = items(mouseEvent->scenePos());
        int itemType = (mouseItems.count() > 0) ? mouseItems.at(0)->type() : -1;

        if((itemType != MapMarkerGraphicsItem::Type) && (isMapMovement(mouseEvent)))
        {
            emit mapMouseRelease(mouseEvent->scenePos());
            mouseEvent->accept();
            return;
        }

        if((mouseEvent->button() == Qt::RightButton) &&
           (mouseEvent->buttonDownScreenPos(Qt::RightButton) == mouseEvent->lastScreenPos()))
        {
            emit addMarker(mouseEvent->scenePos());
            return;
        }
    }

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void MapFrameScene::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{
    if((wheelEvent) &&
       ((wheelEvent->orientation() & Qt::Vertical) == Qt::Vertical) &&
       ((wheelEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
    {
        wheelEvent->accept();
        emit mapZoom(wheelEvent->delta());
    }

    QGraphicsScene::wheelEvent(wheelEvent);
}

void MapFrameScene::keyPressEvent(QKeyEvent *keyEvent)
{
    if((keyEvent) && (keyEvent->key() == Qt::Key_Space))
        _spaceDown = true;

    QGraphicsScene::keyPressEvent(keyEvent);
}

void MapFrameScene::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if((keyEvent) && (keyEvent->key() == Qt::Key_Space))
        _spaceDown = false;

    QGraphicsScene::keyReleaseEvent(keyEvent);
}

bool MapFrameScene::isMapMovement(QGraphicsSceneMouseEvent* mouseEvent)
{
    if(!mouseEvent)
        return false;

    return((_spaceDown) ||
           ((mouseEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) ||
           ((mouseEvent->buttons() & Qt::MiddleButton) == Qt::MiddleButton));
}
