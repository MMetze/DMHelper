#include "unselectedpixmap.h"
#include "battledialogmodel.h"
#include "battledialogmodelcombatant.h"
#include "battleframe.h"
#include "layertokens.h"
#include "layergrid.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

#ifdef DEBUG_FILL_BOUNDING_RECTS
    #include <QPainter>
#endif

UnselectedPixmap::UnselectedPixmap(BattleDialogModelObject* object, QGraphicsItem *parent) :
    QGraphicsPixmapItem(parent),
    _object(object),
    _draw(true)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

UnselectedPixmap::UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelObject* object, QGraphicsItem *parent) :
    QGraphicsPixmapItem(pixmap, parent),
    _object(object),
    _draw(true)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void UnselectedPixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(_draw)
    {
        QStyleOptionGraphicsItem myoption = (*option);
#ifndef DEBUG_FILL_BOUNDING_RECTS
        myoption.state &= ~QStyle::State_Selected;
#endif
        QGraphicsPixmapItem::paint(painter, &myoption, widget);

#ifdef DEBUG_FILL_BOUNDING_RECTS
        if(painter)
            painter->fillRect(boundingRect(), Qt::red);
#endif
    }
}

void UnselectedPixmap::setDraw(bool draw)
{
    _draw = draw;
}

BattleDialogModelObject* UnselectedPixmap::getObject()
{
    return _object;
}

QVariant UnselectedPixmap::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemPositionChange) && (scene()))
    {
        scene()->update(mapRectToScene(boundingRect() | childrenBoundingRect()));

        // Ensure the new position is within the scene rect
        QPointF newPos = mapToScene(mapFromParent(value.toPointF()));
        QRectF rect = scene()->sceneRect();
        bool posOutOfBounds = !rect.contains(newPos);
        if(posOutOfBounds)
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return mapToParent(mapFromScene(newPos));
        }

        // Check if snap to grid is activated for this layer
        if((_object) && (_object->getLayer()))
        {
            LayerTokens* tokenLayer = _object->getLayer();
            BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
            if((battleScene) && (battleScene->getModel()))
            {
                LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(battleScene->getModel()->getLayerScene().getNearest(tokenLayer, DMHelper::LayerType_Grid));
                if((gridLayer) && (gridLayer->getConfig().isSnapToGrid()))
                {
                    // Snap the current position to the grid
                    int intGridSize = static_cast<int>(tokenLayer->getScale());
                    BattleDialogModelCombatant* combatant = dynamic_cast<BattleDialogModelCombatant*>(_object);
                    qreal sizeFactor = combatant ? combatant->getSizeFactor() : 1.0;
                    if(sizeFactor < 1.0)
                    {
                        // For smaller combatants, snap to a grid of half the normal size
                        newPos.setX((static_cast<qreal>(static_cast<int>(newPos.x()) / (intGridSize / 2)) * (tokenLayer->getScale() / 2.0)) + (tokenLayer->getScale() / 4.0));
                        newPos.setY((static_cast<qreal>(static_cast<int>(newPos.y()) / (intGridSize / 2)) * (tokenLayer->getScale() / 2.0)) + (tokenLayer->getScale() / 4.0));
                    }
                    else if(static_cast<int>(sizeFactor) % 2 == 1)
                    {
                        // For combatants that should be centered in the middle of a square
                        newPos.setX((static_cast<qreal>(static_cast<int>(newPos.x()) / intGridSize) * tokenLayer->getScale()) + (tokenLayer->getScale() / 2.0));
                        newPos.setY((static_cast<qreal>(static_cast<int>(newPos.y()) / intGridSize) * tokenLayer->getScale()) + (tokenLayer->getScale() / 2.0));
                    }
                    else
                    {
                        // For combatants that should be centered on a grid crossing
                        newPos.setX((static_cast<qreal>(static_cast<int>(newPos.x()) / intGridSize) * tokenLayer->getScale()));
                        newPos.setY((static_cast<qreal>(static_cast<int>(newPos.y()) / intGridSize) * tokenLayer->getScale()));
                    }
                    return mapToParent(mapFromScene(newPos));
                }
            }
        }
    }
    else if(change == ItemScenePositionHasChanged)
    {
        if(_object)
            _object->setPosition(value.toPointF());
    }
    else if(change == ItemSelectedHasChanged)
    {
        selectionChanged();
    }

    return QGraphicsItem::itemChange(change, value);
}

void UnselectedPixmap::selectionChanged()
{
    if(!scene())
        return;

    if((_object) && (_object->getObjectType() != DMHelper::CampaignType_BattleContentCombatant))
        return;

    if(isSelected())
    {
        BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
        if(!battleScene)
            return;

        QPixmap pmp = battleScene->getSelectedIcon();
        QGraphicsPixmapItem* selectedPixmap = scene()->addPixmap(pmp.scaled(boundingRect().size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        if(!selectedPixmap)
            return;

        selectedPixmap->setTransformationMode(Qt::SmoothTransformation);

        QSizeF pixmapSize = selectedPixmap->boundingRect().size();
        selectedPixmap->setPos(-pixmapSize.width() / 2.0, -pixmapSize.height() / 2.0);

        selectedPixmap->setData(BATTLE_CONTENT_CHILD_INDEX, BattleDialogItemChild_Selection);
        selectedPixmap->setParentItem(this);
    }
    else
    {
        for(QGraphicsItem* childItem : childItems())
        {
            if((childItem) && (childItem->data(BATTLE_CONTENT_CHILD_INDEX).toInt() == BattleDialogItemChild_Selection))
            {
                childItem->setParentItem(nullptr);
                delete childItem;
            }
        }
    }

    if(_object)
        _object->setSelected(isSelected());
}
