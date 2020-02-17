#include "unselectedpixmap.h"
#include "battledialogmodel.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>

UnselectedPixmap::UnselectedPixmap(BattleDialogModelCombatant* combatant) :
    QGraphicsPixmapItem(),
    _combatant(combatant),
    _draw(true)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    //setAcceptHoverEvents(true);
}

UnselectedPixmap::UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelCombatant* combatant) :
    QGraphicsPixmapItem(pixmap),
    _combatant(combatant),
    _draw(true)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void UnselectedPixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if(_draw)
    {
        QStyleOptionGraphicsItem myoption = (*option);
        myoption.state &= ~QStyle::State_Selected;
        QGraphicsPixmapItem::paint(painter, &myoption, widget);
    }
}

void UnselectedPixmap::setDraw(bool draw)
{
    _draw = draw;
}

/*
void UnselectedPixmap::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if((!event) || ((flags() & QGraphicsItem::ItemIsSelectable) == 0))
    {
        unsetCursor();
        return;
    }

    setCursor(Qt::ArrowCursor);
}
*/

QVariant UnselectedPixmap::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if((change == ItemPositionChange) && (scene()))
    {
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        bool posOutOfBounds = !rect.contains(newPos);
        if(posOutOfBounds)
        {
            newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
            newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
            return newPos;
        }
    }
    else if(change == ItemPositionHasChanged)
    {
        if(_combatant)
        {
            QPointF newPos = value.toPointF();
            _combatant->setPosition(newPos);
        }
    }

    return QGraphicsItem::itemChange(change, value);
}
