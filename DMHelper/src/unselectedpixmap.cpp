#include "unselectedpixmap.h"
#include "battledialogmodel.h"
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QDebug>

UnselectedPixmap::UnselectedPixmap(BattleDialogModelCombatant* combatant) :
    QGraphicsPixmapItem(),
    _combatant(combatant)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

UnselectedPixmap::UnselectedPixmap(const QPixmap &pixmap, BattleDialogModelCombatant* combatant) :
    QGraphicsPixmapItem(pixmap),
    _combatant(combatant)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
}

void UnselectedPixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myoption = (*option);
    myoption.state &= ~QStyle::State_Selected;
    QGraphicsPixmapItem::paint(painter, &myoption, widget);
}

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
        }

        if(_combatant)
            _combatant->setPosition(newPos);

        //qDebug() << "[UnselectedPixmap] item change identified: " << newPos << ", out of bounds: " << posOutOfBounds;

        if(posOutOfBounds)
            return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}
