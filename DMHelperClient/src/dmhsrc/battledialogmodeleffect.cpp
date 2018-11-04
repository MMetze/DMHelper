#include "battledialogmodeleffect.h"
#include "dmconstants.h"
#include <QDebug>
#include <QDomElement>
#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QBrush>

BattleDialogModelEffect::BattleDialogModelEffect() :
    DMHObjectBase(),
    _size(20),
    _position(0,0),
    _rotation(0),
    _color(115,18,0),
    _tip()
{
}

BattleDialogModelEffect::BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    DMHObjectBase(),
    _size(size),
    _position(position),
    _rotation(rotation),
    _color(color),
    _tip(tip)
{
}

BattleDialogModelEffect::BattleDialogModelEffect(const BattleDialogModelEffect& other) :
    DMHObjectBase(other),
    _size(other._size),
    _position(other._position),
    _rotation(other._rotation),
    _color(other._color),
    _tip(other._tip)
{
}

BattleDialogModelEffect::~BattleDialogModelEffect()
{
}

/*
void BattleDialogModelEffect::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    QDomElement element = doc.createElement( "battleeffect" );

    CampaignObjectBase::outputXML(doc, element, targetDirectory);

    element.setAttribute("type", getType());
    element.setAttribute("size", _size);
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());
    element.setAttribute("rotation", _rotation );
    element.setAttribute("colorR", _color.red());
    element.setAttribute("colorG", _color.green());
    element.setAttribute("colorB", _color.blue());
    element.setAttribute("tip", _tip);

    parent.appendChild(element);
}
*/

void BattleDialogModelEffect::inputXML(const QDomElement &element)
{
    DMHObjectBase::inputXML(element);

    _size = element.attribute("size",QString::number(20)).toInt();
    _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                        element.attribute("positionY",QString::number(0)).toDouble());
    _rotation = element.attribute("rotation",QString::number(0)).toDouble();
    _color = QColor(element.attribute("colorR",QString::number(255)).toInt(),
                    element.attribute("colorG",QString::number(255)).toInt(),
                    element.attribute("colorB",QString::number(255)).toInt());
    _tip = element.attribute("tip");
}

void BattleDialogModelEffect::applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const
{
    qDebug() << "[Battle Dialog Model Effect] applying effect values for " << this << " to " << &item;
    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setPen(QPen(getColor(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item.setBrush(QBrush(QColor(getColor().red(),getColor().green(),getColor().blue(),64)));
    item.setToolTip(getTip());

    item.setScale((qreal)getSize() * gridScale / 500.0);
}

int BattleDialogModelEffect::getSize() const
{
    return _size;
}

void BattleDialogModelEffect::setSize(int size)
{
    _size = size;
}

QPointF BattleDialogModelEffect::getPosition() const
{
    return _position;
}

void BattleDialogModelEffect::setPosition(const QPointF& position)
{
    _position = position;
}

void BattleDialogModelEffect::setPosition(qreal x, qreal y)
{
    setPosition(QPointF(x, y));
}

qreal BattleDialogModelEffect::getRotation() const
{
    return _rotation;
}

void BattleDialogModelEffect::setRotation(qreal rotation)
{
    _rotation = rotation;
}

QColor BattleDialogModelEffect::getColor() const
{
    return _color;
}

void BattleDialogModelEffect::setColor(const QColor& color)
{
    _color = color;
}

QString BattleDialogModelEffect::getTip() const
{
    return _tip;
}

void BattleDialogModelEffect::setTip(const QString& tip)
{
    _tip = tip;
}

void BattleDialogModelEffect::prepareItem(QAbstractGraphicsShapeItem& item) const
{
    item.setFlag(QGraphicsItem::ItemIsMovable, true);
    item.setFlag(QGraphicsItem::ItemIsSelectable, true);
    item.setZValue(DMHelper::BattleDialog_Z_BackHighlight);
}
