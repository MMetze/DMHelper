#include "unselectedpixmap.h"
#include "battledialogmodel.h"
#include "battleframe.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>

UnselectedPixmap::UnselectedPixmap(BattleDialogModelObject* object, QGraphicsItem *parent) :
    QGraphicsPixmapItem(parent),
    _object(object),
    _draw(true)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    //setAcceptHoverEvents(true);
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
        //myoption.state &= ~QStyle::State_Selected;
        QGraphicsPixmapItem::paint(painter, &myoption, widget);
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
        QPointF newPos = mapToScene(value.toPointF());
        QRectF rect = scene()->sceneRect();
        bool posOutOfBounds = !rect.contains(newPos);
        if(posOutOfBounds)
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return mapFromScene(newPos);
        }
    }
    /*
    else if(change == ItemPositionHasChanged)
    {
        if(_combatant)
        {
            QPointF newPos = value.toPointF();
            _combatant->setPosition(newPos);
        }
    }
    */
    else if(change == ItemScenePositionHasChanged)
    {
        if(_object)
        {
            QPointF newPos = value.toPointF();
            _object->setPosition(newPos);
        }
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

    if(isSelected())
    {
        BattleDialogGraphicsScene* battleScene = dynamic_cast<BattleDialogGraphicsScene*>(scene());
        if(!battleScene)
            return;

        QPixmap pmp = battleScene->getSelectedIcon();
        //QPixmap pmp(":/img/data/selected.png");
        //QPixmap pmp(":/img/data/icon_square.png");
        QGraphicsPixmapItem* selectedPixmap = scene()->addPixmap(pmp.scaled(boundingRect().size().toSize(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        if(!selectedPixmap)
            return;

        selectedPixmap->setTransformationMode(Qt::SmoothTransformation);
        //selectedPixmap->setScale(scale()); //(static_cast<qreal>(_model->getGridScale()) * _selectedScale / ACTIVE_PIXMAP_SIZE);
        selectedPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);

        // QRect itemRect = item->boundingRect().toRect();
        // int maxSize = qMax(itemRect.width(), itemRect.height());

        QSizeF pixmapSize = selectedPixmap->boundingRect().size();
        selectedPixmap->setPos(-pixmapSize.width() / 2.0, -pixmapSize.height() / 2.0);

        selectedPixmap->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent);
        selectedPixmap->setZValue(DMHelper::BattleDialog_Z_FrontHighlight);
        selectedPixmap->setData(BattleDialogItemChild_Index, BattleFrame::BattleDialogItemChild_Selection);
        selectedPixmap->setParentItem(this);
    }
    else
    {
        for(QGraphicsItem* childItem : childItems())
        {
            if((childItem) && (childItem->data(BattleDialogItemChild_Index).toInt() == BattleFrame::BattleDialogItemChild_Selection))
            {
                childItem->setParentItem(nullptr);
                delete childItem;
            }
        }
    }

    if(_object)
        _object->setSelected(isSelected());
}
