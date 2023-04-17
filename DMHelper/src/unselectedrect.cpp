#include "unselectedrect.h"
#include "battledialogmodelobject.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

UnselectedRect::UnselectedRect(BattleDialogModelObject* object, qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent) :
    QGraphicsRectItem(x, y, width, height, parent),
    _object(object)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void UnselectedRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsRectItem::paint(painter, &myoption, widget);
}

QVariant UnselectedRect::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemSelectedHasChanged)
    {
        QPen itemPen = pen();
        itemPen.setWidth(isSelected() ? 3 : 1);
        setPen(itemPen);
    }
    else if(change == ItemScenePositionHasChanged)
    {
        if(_object)
        {
            QPointF newPos = value.toPointF();
            _object->setPosition(newPos);
        }
    }

    return QGraphicsItem::itemChange(change, value);
}
