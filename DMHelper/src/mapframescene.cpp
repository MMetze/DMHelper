#include "mapframescene.h"
#include "mapmarkergraphicsitem.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>

MapFrameScene::MapFrameScene(QObject* parent) :
    QGraphicsScene(parent),
    _spaceDown(false),
    _contextMenuItem(nullptr),
    _contextMenuPos()
{
}

void MapFrameScene::handleAddMarker()
{
    emit addMarker(_contextMenuPos);
}

void MapFrameScene::handleEditMarker()
{
    if(!_contextMenuItem)
        return;

    MapMarkerGraphicsItem* markerItem = dynamic_cast<MapMarkerGraphicsItem*>(_contextMenuItem);
    if((!markerItem) && (_contextMenuItem->parentItem()))
        markerItem = dynamic_cast<MapMarkerGraphicsItem*>(_contextMenuItem->parentItem());

    if(markerItem)
        emit editMarker(markerItem->getMarkerId());
}

void MapFrameScene::handleDeleteMarker()
{
    if(!_contextMenuItem)
        return;

    MapMarkerGraphicsItem* markerItem = dynamic_cast<MapMarkerGraphicsItem*>(_contextMenuItem);
    if((!markerItem) && (_contextMenuItem->parentItem()))
        markerItem = dynamic_cast<MapMarkerGraphicsItem*>(_contextMenuItem->parentItem());

    if(markerItem)
        emit deleteMarker(markerItem->getMarkerId());
}

void MapFrameScene::handleCenterView()
{
    emit centerView(_contextMenuPos);
}

void MapFrameScene::handleClearFoW()
{
    emit clearFoW();
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
            QMenu menu(views().constFirst());
            _contextMenuPos = mouseEvent->scenePos();
            if(itemType == MapMarkerGraphicsItem::Type)
            {
                _contextMenuItem = mouseItems.at(0);

                QAction* editMarkerAction = new QAction(QString("Edit Marker..."), &menu);
                connect(editMarkerAction, SIGNAL(triggered()), this, SLOT(handleEditMarker()));
                menu.addAction(editMarkerAction);

                QAction* deleteMarkerAction = new QAction(QString("Delete Marker..."), &menu);
                connect(deleteMarkerAction, SIGNAL(triggered()), this, SLOT(handleDeleteMarker()));
                menu.addAction(deleteMarkerAction);
            }
            else
            {
                QAction* addMarkerAction = new QAction(QString("Add Marker..."), &menu);
                connect(addMarkerAction, SIGNAL(triggered()), this, SLOT(handleAddMarker()));
                menu.addAction(addMarkerAction);
            }

            menu.addSeparator();

            QAction* centerViewAction = new QAction(QString("Center View"), &menu);
            connect(centerViewAction, SIGNAL(triggered()), this, SLOT(centerView()));
            menu.addAction(centerViewAction);

            QAction* clearFoWAction = new QAction(QString("Clear Fog of War"), &menu);
            connect(clearFoWAction, SIGNAL(triggered()), this, SLOT(handleClearFoW()));
            menu.addAction(clearFoWAction);

            menu.exec(mouseEvent->screenPos());
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
