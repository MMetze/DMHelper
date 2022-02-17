#include "battledialogmodeleffectshape.h"
#include <QDomElement>
#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QBrush>

BattleDialogModelEffectShape::BattleDialogModelEffectShape(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent)
{
}

BattleDialogModelEffectShape::BattleDialogModelEffectShape(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip)
{
}

BattleDialogModelEffectShape::~BattleDialogModelEffectShape()
{
}

void BattleDialogModelEffectShape::applyEffectValues(QGraphicsItem& item, qreal gridScale) const
{
    BattleDialogModelEffect::applyEffectValues(item, gridScale);

    QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(&item);
    if(shapeItem)
    {
        shapeItem->setPen(QPen(QColor(getColor().red(),getColor().green(),getColor().blue(),255), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        shapeItem->setBrush(QBrush(getColor()));
    }
}
