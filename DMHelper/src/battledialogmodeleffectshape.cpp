#include "battledialogmodeleffectshape.h"
#include <QDomElement>
#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QBrush>

BattleDialogModelEffectShape::BattleDialogModelEffectShape(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _color(115,18,0,64)
{
}

BattleDialogModelEffectShape::BattleDialogModelEffectShape(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, tip),
    _color(color)
{
}

BattleDialogModelEffectShape::~BattleDialogModelEffectShape()
{
}

// From CampaignObjectBase
void BattleDialogModelEffectShape::inputXML(const QDomElement &element, bool isImport)
{
    _color = QColor(element.attribute("colorR",QString::number(255)).toInt(),
                    element.attribute("colorG",QString::number(255)).toInt(),
                    element.attribute("colorB",QString::number(255)).toInt(),
                    element.attribute("colorA",QString::number(64)).toInt());

    BattleDialogModelEffect::inputXML(element, isImport);
}

void BattleDialogModelEffectShape::applyEffectValues(QGraphicsItem& item, qreal gridScale) const
{
    BattleDialogModelEffect::applyEffectValues(item, gridScale);

    QAbstractGraphicsShapeItem* shapeItem = dynamic_cast<QAbstractGraphicsShapeItem*>(&item);
    if(shapeItem)
    {
        //shapeItem->setPen(QPen(getColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        shapeItem->setPen(QPen(QColor(getColor().red(),getColor().green(),getColor().blue(),255), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        //shapeItem->setBrush(QBrush(QColor(getColor().red(),getColor().green(),getColor().blue(),getColor().alpha())));
        shapeItem->setBrush(QBrush(getColor()));
    }
}

QColor BattleDialogModelEffectShape::getColor() const
{
    return _color;
}

void BattleDialogModelEffectShape::setColor(const QColor& color)
{
    _color = color;
}

void BattleDialogModelEffectShape::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("colorR", _color.red());
    element.setAttribute("colorG", _color.green());
    element.setAttribute("colorB", _color.blue());
    element.setAttribute("colorA", _color.alpha());

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}

void BattleDialogModelEffectShape::copyValues(const BattleDialogModelEffectShape &other)
{
    BattleDialogModelEffect::copyValues(other);
    _color = other._color;
}
