#include "battledialogmodeleffectcone.h"
#include <QGraphicsPolygonItem>

BattleDialogModelEffectCone::BattleDialogModelEffectCone() :
    BattleDialogModelEffect()
{
}

BattleDialogModelEffectCone::BattleDialogModelEffectCone(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectCone::BattleDialogModelEffectCone(const BattleDialogModelEffectCone& other) :
    BattleDialogModelEffect(other)
{
}

BattleDialogModelEffectCone::~BattleDialogModelEffectCone()
{
}

BattleDialogModelEffect* BattleDialogModelEffectCone::clone() const
{
    return new BattleDialogModelEffectCone(*this);
}

int BattleDialogModelEffectCone::getType() const
{
    return BattleDialogModelEffect_Cone;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectCone::createEffectShape(qreal gridScale) const
{
    QPolygonF poly;
    poly << QPointF(0,0) << QPointF(-50,100) << QPoint(50,100) << QPoint(0,0);

    QGraphicsPolygonItem* triangleItem = new QGraphicsPolygonItem(poly);
    triangleItem->setData(BATTLE_DIALOG_MODEL_EFFECT_ID, getID().toString());
    prepareItem(*triangleItem);
    applyEffectValues(*triangleItem, gridScale);

    return triangleItem;
}
