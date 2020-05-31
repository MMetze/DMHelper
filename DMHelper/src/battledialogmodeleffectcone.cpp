#include "battledialogmodeleffectcone.h"
#include "unselectedpolygon.h"

BattleDialogModelEffectCone::BattleDialogModelEffectCone(const QString& name, QObject *parent) :
    BattleDialogModelEffectShape(name, parent)
{
}

BattleDialogModelEffectCone::BattleDialogModelEffectCone(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffectShape(size, position, rotation, color, tip)
{
}

/*
BattleDialogModelEffectCone::BattleDialogModelEffectCone(const BattleDialogModelEffectCone& other) :
    BattleDialogModelEffect(other)
{
}
*/

BattleDialogModelEffectCone::~BattleDialogModelEffectCone()
{
}

BattleDialogModelEffect* BattleDialogModelEffectCone::clone() const
{
    BattleDialogModelEffectCone* newEffect = new BattleDialogModelEffectCone(getName());
    newEffect->copyValues(*this);
    return newEffect;
}

int BattleDialogModelEffectCone::getEffectType() const
{
    return BattleDialogModelEffect_Cone;
}

QGraphicsItem* BattleDialogModelEffectCone::createEffectShape(qreal gridScale) const
{
    QPolygonF poly;
    poly << QPointF(0,0) << QPointF(-50,100) << QPoint(50,100) << QPoint(0,0);

    QGraphicsPolygonItem* triangleItem = new UnselectedPolygon(poly);
    setEffectItemData(triangleItem);

    prepareItem(*triangleItem);
    applyEffectValues(*triangleItem, gridScale);

    return triangleItem;
}
