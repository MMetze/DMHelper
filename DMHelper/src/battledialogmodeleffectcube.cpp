#include "battledialogmodeleffectcube.h"
#include "unselectedrect.h"

BattleDialogModelEffectCube::BattleDialogModelEffectCube(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent)
{
}
BattleDialogModelEffectCube::BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

/*
BattleDialogModelEffectCube::BattleDialogModelEffectCube(const BattleDialogModelEffectCube& other) :
    BattleDialogModelEffect(other)
{
}
*/

BattleDialogModelEffectCube::~BattleDialogModelEffectCube()
{
}

BattleDialogModelEffect* BattleDialogModelEffectCube::clone() const
{
    BattleDialogModelEffectCube* newEffect = new BattleDialogModelEffectCube(getName());
    newEffect->copyValues(*this);
    return newEffect;
}

int BattleDialogModelEffectCube::getEffectType() const
{
    return BattleDialogModelEffect_Cube;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectCube::createEffectShape(qreal gridScale) const
{
    QGraphicsRectItem* rectItem = new UnselectedRect(0,0,100,100);

    setEffectItemData(rectItem);

    prepareItem(*rectItem);
    applyEffectValues(*rectItem, gridScale);

    return rectItem;
}
