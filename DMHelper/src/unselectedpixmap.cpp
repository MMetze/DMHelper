#include "unselectedpixmap.h"
#include "battledialogmodel.h"
#include "battleframe.h"
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

        QPointF newPos = mapToScene(mapFromParent(value.toPointF()));
        QRectF rect = scene()->sceneRect();
        bool posOutOfBounds = !rect.contains(newPos);
        if(posOutOfBounds)
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return mapToParent(mapFromScene(newPos));
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
