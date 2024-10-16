#include "battledialogmodeleffectradius.h"
#include "battledialogeffectsettings.h"
#include "unselectedellipse.h"

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(const QString& name, QObject *parent) :
    BattleDialogModelEffectShape(name, parent)
{
}

BattleDialogModelEffectRadius::BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffectShape(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectRadius::~BattleDialogModelEffectRadius()
{
}

QString BattleDialogModelEffectRadius::getName() const
{
    return _tip.isEmpty() ? QString("Radius Effect") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectRadius::clone() const
{
    BattleDialogModelEffectRadius* newEffect = new BattleDialogModelEffectRadius(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectRadius::getEffectType() const
{
    return BattleDialogModelEffect_Radius;
}

BattleDialogEffectSettingsBase* BattleDialogModelEffectRadius::getEffectEditor() const
{
    BattleDialogEffectSettings* result = new BattleDialogEffectSettings(*this);
    result->setSizeLabel(QString("Radius"));
    return result;
}

QGraphicsItem* BattleDialogModelEffectRadius::createEffectShape(qreal gridScale)
{
    QGraphicsEllipseItem* circleItem = new UnselectedEllipse(this, -100, -100, 200, 200);

    setEffectItemData(circleItem);

    prepareItem(*circleItem);
    applyEffectValues(*circleItem, gridScale);

    return circleItem;
}
