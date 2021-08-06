#include "mapframescene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
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
    }
    else
    {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void MapFrameScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(isMapMovement(mouseEvent))
    {
        emit mapMousePress(mouseEvent->scenePos());
        mouseEvent->accept();
    }
    else
    {
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
}

void MapFrameScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if(isMapMovement(mouseEvent))
    {
        emit mapMouseRelease(mouseEvent->scenePos());
        mouseEvent->accept();
    }
    else
    {
        QGraphicsScene::mouseReleaseEvent(mouseEvent);
    }
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
