#include "battledialogmodeleffectcube.h"
#include "unselectedrect.h"

BattleDialogModelEffectCube::BattleDialogModelEffectCube() :
    BattleDialogModelEffect()
{
}
BattleDialogModelEffectCube::BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectCube::BattleDialogModelEffectCube(const BattleDialogModelEffectCube& other) :
    BattleDialogModelEffect(other)
{
}

BattleDialogModelEffectCube::~BattleDialogModelEffectCube()
{
}

BattleDialogModelEffect* BattleDialogModelEffectCube::clone() const
{
    return new BattleDialogModelEffectCube(*this);
}

int BattleDialogModelEffectCube::getType() const
{
    return BattleDialogModelEffect_Cube;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectCube::createEffectShape(qreal gridScale) const
{
    QGraphicsRectItem* rectItem = new UnselectedRect(0,0,100,100);

    rectItem->setData(BATTLE_DIALOG_MODEL_EFFECT_ID, getID().toString());
    //qreal scaledSize = (qreal)effect->getSize() * _model.getGridScale() / 5.f;

    prepareItem(*rectItem);
    applyEffectValues(*rectItem, gridScale);

    return rectItem;
}
