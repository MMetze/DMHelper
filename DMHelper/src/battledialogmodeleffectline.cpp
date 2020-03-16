#include "battledialogmodeleffectline.h"
#include <unselectedrect.h>
#include <QDebug>
#include <QPen>

BattleDialogModelEffectLine::BattleDialogModelEffectLine() :
    BattleDialogModelEffect()
{
}

BattleDialogModelEffectLine::BattleDialogModelEffectLine(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectLine::BattleDialogModelEffectLine(const BattleDialogModelEffectLine& other) :
    BattleDialogModelEffect(other)
{
}

BattleDialogModelEffectLine::~BattleDialogModelEffectLine()
{
}

BattleDialogModelEffect* BattleDialogModelEffectLine::clone() const
{
    return new BattleDialogModelEffectLine(*this);
}

int BattleDialogModelEffectLine::getType() const
{
    return BattleDialogModelEffect_Line;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectLine::createEffectShape(qreal gridScale) const
{
    QGraphicsRectItem* rectItem = new UnselectedRect(-250.0, 0.0, 500.0, (qreal)getSize() * 100.f);

    rectItem->setData(BATTLE_DIALOG_MODEL_EFFECT_ID, getID().toString());
    //qreal scaledSize = _model.getGridScale() / 500.f;

    prepareItem(*rectItem);
    applyEffectValues(*rectItem, gridScale);

    return rectItem;
}

void BattleDialogModelEffectLine::applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const
{
    // First apply the base information
    BattleDialogModelEffect::applyEffectValues(item, gridScale);

    qDebug() << "[Battle Dialog Model Effect Line] applying extra line effect values...";

    // Now correct the special case information for the line effect
    QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(&item);
    if(rectItem)
        rectItem->setRect(-250.0, 0.0, 500.0, (qreal)getSize() * 100.0);
    else
        qDebug() << "[Battle Dialog Model Effect Line] ERROR: Line Effect found without QGraphicsRectItem!";

    item.setScale(gridScale / 500.0);
    item.setPen(QPen(getColor(), 10, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}
