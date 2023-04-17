#include "unselectedpolygon.h"
#include "battledialogmodelobject.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

UnselectedPolygon::UnselectedPolygon(BattleDialogModelObject* object, const QPolygonF &polygon, QGraphicsItem *parent) :
    QGraphicsPolygonItem(polygon, parent),
    _object(object)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void UnselectedPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsPolygonItem::paint(painter, &myoption, widget);
}

QVariant UnselectedPolygon::itemChange(GraphicsItemChange change, const QVariant &value)
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
