#include "battledialogmodeleffectcube.h"
#include "unselectedrect.h"

BattleDialogModelEffectCube::BattleDialogModelEffectCube(const QString& name, QObject *parent) :
    BattleDialogModelEffectShape(name, parent)
{
}
BattleDialogModelEffectCube::BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffectShape(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectCube::~BattleDialogModelEffectCube()
{
}

BattleDialogModelEffect* BattleDialogModelEffectCube::clone() const
{
    BattleDialogModelEffectCube* newEffect = new BattleDialogModelEffectCube(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectCube::getEffectType() const
{
    return BattleDialogModelEffect_Cube;
}

QGraphicsItem* BattleDialogModelEffectCube::createEffectShape(qreal gridScale)
{
    QGraphicsRectItem* rectItem = new UnselectedRect(0,0,100,100);

    setEffectItemData(rectItem);

    prepareItem(*rectItem);
    applyEffectValues(*rectItem, gridScale);

    return rectItem;
}
