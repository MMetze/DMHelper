#include "battledialogmodeleffect.h"
#include "battledialogeffectsettings.h"
#include "dmconstants.h"
#include <QDebug>
#include <QDomElement>
#include <QGraphicsItem>
#include <QVariant>

BattleDialogModelEffect::BattleDialogModelEffect(const QString& name, QObject *parent) :
    CampaignObjectBase(name, parent),
    _active(true),
    _visible(true),
    _size(20),
    _position(0,0),
    _rotation(0),
    _color(115,18,0,64),
    _tip()
{
}

BattleDialogModelEffect::BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    CampaignObjectBase(),
    _active(true),
    _visible(true),
    _size(size),
    _position(position),
    _rotation(rotation),
    _color(color),
    _tip(tip)
{
}

BattleDialogModelEffect::~BattleDialogModelEffect()
{
}

void BattleDialogModelEffect::inputXML(const QDomElement &element, bool isImport)
{
    _active = static_cast<bool>(element.attribute("active",QString::number(1)).toInt());
    _visible = static_cast<bool>(element.attribute("visible",QString::number(1)).toInt());
    _size = element.attribute("size",QString::number(20)).toInt();
    _position = QPointF(element.attribute("positionX",QString::number(0)).toDouble(),
                        element.attribute("positionY",QString::number(0)).toDouble());
    _rotation = element.attribute("rotation",QString::number(0)).toDouble();
    _color = QColor(element.attribute("colorR",QString::number(115)).toInt(),
                    element.attribute("colorG",QString::number(18)).toInt(),
                    element.attribute("colorB",QString::number(0)).toInt(),
                    element.attribute("colorA",QString::number(64)).toInt());
    _tip = element.attribute("tip");

    CampaignObjectBase::inputXML(element, isImport);
}

BattleDialogEffectSettings* BattleDialogModelEffect::getEffectEditor() const
{
    return new BattleDialogEffectSettings(*this);
}

void BattleDialogModelEffect::applyEffectValues(QGraphicsItem& item, qreal gridScale) const
{
    qDebug() << "[Battle Dialog Model Effect] applying effect values for " << this << " to " << &item;
    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setToolTip(getTip());

    item.setScale(static_cast<qreal>(getSize()) * gridScale / 500.0);
}

bool BattleDialogModelEffect::getEffectActive() const
{
    return _active;
}

void BattleDialogModelEffect::setEffectActive(bool active)
{
    _active = active;
}

bool BattleDialogModelEffect::getEffectVisible() const
{
    return _visible;
}

void BattleDialogModelEffect::setEffectVisible(bool visible)
{
    _visible = visible;
}

int BattleDialogModelEffect::getSize() const
{
    return _size;
}

void BattleDialogModelEffect::setSize(int size)
{
    _size = size;
}

int BattleDialogModelEffect::getWidth() const
{
    return -1;
}

void BattleDialogModelEffect::setWidth(int width)
{
    Q_UNUSED(width);
}

void BattleDialogModelEffect::setItemScale(QGraphicsItem* item, qreal scaleFactor) const
{
    if(item)
        item->setScale(scaleFactor);
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

int BattleDialogModelEffect::getImageRotation() const
{
    return 0;
}

void BattleDialogModelEffect::setImageRotation(int imageRotation)
{
    Q_UNUSED(imageRotation);
}

QString BattleDialogModelEffect::getImageFile() const
{
    return QString();
}

void BattleDialogModelEffect::setImageFile(const QString& imageFile)
{
    Q_UNUSED(imageFile);
}

QString BattleDialogModelEffect::getTip() const
{
    return _tip;
}

void BattleDialogModelEffect::setTip(const QString& tip)
{
    _tip = tip;
}

void BattleDialogModelEffect::setEffectItemData(QGraphicsItem* item) const
{
    if(!item)
        return;

    item->setData(BATTLE_DIALOG_MODEL_EFFECT_ID, getID().toString());
    item->setData(BATTLE_DIALOG_MODEL_EFFECT_OBJECT, QVariant::fromValue(reinterpret_cast<quint64>(this)));
}

QUuid BattleDialogModelEffect::getEffectIdFromItem(QGraphicsItem* item)
{
    if(!item)
        return QUuid();

    return QUuid(item->data(BATTLE_DIALOG_MODEL_EFFECT_ID).toString());
}

BattleDialogModelEffect* BattleDialogModelEffect::getEffectFromItem(QGraphicsItem* item)
{
    if(!item)
        return nullptr;

    return reinterpret_cast<BattleDialogModelEffect*>(item->data(BATTLE_DIALOG_MODEL_EFFECT_OBJECT).value<quint64>());
}

bool BattleDialogModelEffect::getEffectActiveFromItem(QGraphicsItem* item)
{
    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
    return (effect ? effect->getEffectActive() : false);
}

bool BattleDialogModelEffect::getEffectVisibleFromItem(QGraphicsItem* item)
{
    BattleDialogModelEffect* effect = BattleDialogModelEffect::getEffectFromItem(item);
    return (effect ? effect->getEffectVisible() : false);
}

QDomElement BattleDialogModelEffect::createOutputXML(QDomDocument &doc)
{
    return doc.createElement( "battleeffect" );
}

void BattleDialogModelEffect::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getEffectType());
    element.setAttribute("active", _active);
    element.setAttribute("visible", _visible);
    element.setAttribute("size", _size);
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());
    element.setAttribute("rotation", _rotation );
    element.setAttribute("colorR", _color.red());
    element.setAttribute("colorG", _color.green());
    element.setAttribute("colorB", _color.blue());
    element.setAttribute("colorA", _color.alpha());
    element.setAttribute("tip", _tip);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}

bool BattleDialogModelEffect::belongsToObject(QDomElement& element)
{
    Q_UNUSED(element);

    // Don't auto-input any child objects of the battle. The battle will handle this itself.
    return true;
}

void BattleDialogModelEffect::prepareItem(QGraphicsItem& item) const
{
    item.setFlag(QGraphicsItem::ItemIsMovable, true);
    item.setFlag(QGraphicsItem::ItemIsSelectable, true);
    item.setZValue(DMHelper::BattleDialog_Z_BackHighlight);
}

void BattleDialogModelEffect::copyValues(const BattleDialogModelEffect &other)
{
    _active = other._active;
    _visible = other._visible;
    _size = other._size;
    setWidth(other.getWidth());
    _position = other._position;
    _rotation = other._rotation;
    _color = other._color;
    setImageRotation(other.getImageRotation());
    setImageFile(other.getImageFile());
    _tip = other._tip;
}
