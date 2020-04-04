#include "battledialogmodeleffectradius.h"
#include "unselectedellipse.h"

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent)
{
}

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size * 2, position, rotation, color, tip)
{
}

/*
BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(const BattleDialogModelEffectRadius& other) :
    BattleDialogModelEffect(other)
{
}
*/

BattleDialogModelEffectRadius::~BattleDialogModelEffectRadius()
{
}

BattleDialogModelEffect* BattleDialogModelEffectRadius::clone() const
{
    BattleDialogModelEffectRadius* newEffect = new BattleDialogModelEffectRadius(getName());
    newEffect->copyValues(*this);
    return newEffect;
}

int BattleDialogModelEffectRadius::getEffectType() const
{
    return BattleDialogModelEffect_Radius;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectRadius::createEffectShape(qreal gridScale) const
{
    QGraphicsEllipseItem* circleItem = new UnselectedEllipse(0,0,100,100);

    circleItem->setData(BATTLE_DIALOG_MODEL_EFFECT_ID, getID().toString());
    prepareItem(*circleItem);
    applyEffectValues(*circleItem, gridScale);

    return circleItem;
}
