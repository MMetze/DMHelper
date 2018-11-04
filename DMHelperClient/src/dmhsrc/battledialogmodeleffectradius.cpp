#include "battledialogmodeleffectradius.h"
#include <QGraphicsEllipseItem>

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius() :
    BattleDialogModelEffect()
{
}

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(const BattleDialogModelEffect& other) :
    BattleDialogModelEffect(other)
{
}

BattleDialogModelEffectRadius::~BattleDialogModelEffectRadius()
{
}

BattleDialogModelEffect* BattleDialogModelEffectRadius::clone() const
{
    return new BattleDialogModelEffectRadius(*this);
}

int BattleDialogModelEffectRadius::getType() const
{
    return BattleDialogModelEffect_Radius;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectRadius::createEffectShape(qreal gridScale) const
{
    QGraphicsEllipseItem* circleItem = new QGraphicsEllipseItem(0,0,100,100);

    circleItem->setData(0, getID());
    prepareItem(*circleItem);
    applyEffectValues(*circleItem, gridScale);

    return circleItem;
}
