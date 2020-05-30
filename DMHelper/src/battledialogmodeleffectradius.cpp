#include "battledialogmodeleffectradius.h"
#include "battledialogeffectsettings.h"
#include "unselectedellipse.h"

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent)
{
}

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

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

BattleDialogEffectSettings* BattleDialogModelEffectRadius::getEffectEditor() const
{
    BattleDialogEffectSettings* result = new BattleDialogEffectSettings(*this);
    result->setSizeLabel(QString("Radius"));
    return result;
}

QAbstractGraphicsShapeItem* BattleDialogModelEffectRadius::createEffectShape(qreal gridScale) const
{
    QGraphicsEllipseItem* circleItem = new UnselectedEllipse(0, 0, 200, 200);

    setEffectItemData(circleItem);

    prepareItem(*circleItem);
    applyEffectValues(*circleItem, gridScale);

    return circleItem;
}
