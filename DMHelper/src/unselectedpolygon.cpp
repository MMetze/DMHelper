#include "unselectedpolygon.h"
#include "battledialogmodelobject.h"
#include "dmconstants.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPen>

#ifdef DEBUG_FILL_BOUNDING_RECTS
    #include <QPainter>
#endif

UnselectedPolygon::UnselectedPolygon(BattleDialogModelObject* object, const QPolygonF &polygon, QGraphicsItem *parent) :
    QGraphicsPolygonItem(polygon, parent),
    _object(object)
{
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void UnselectedPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
#ifdef SHOW_UNSELECTED_SHAPE_SELECTION
    QGraphicsPolygonItem::paint(painter, option, widget);
#else
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsPolygonItem::paint(painter, &myoption, widget);
#endif

#ifdef DEBUG_FILL_BOUNDING_RECTS
        if(painter)
            painter->fillRect(boundingRect(), Qt::blue);
#endif
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
