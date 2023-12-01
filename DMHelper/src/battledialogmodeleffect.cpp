#include "battledialogmodeleffect.h"
#include "battledialogeffectsettings.h"
#include "dmconstants.h"
#include <QDebug>
#include <QDomElement>
#include <QGraphicsItem>
#include <QVariant>

BattleDialogModelEffect::BattleDialogModelEffect(const QString& name, QObject *parent) :
    BattleDialogModelObject(QPointF(), name, parent),
    _tokensLayer(nullptr),
    _active(true),
    _visible(true),
    _size(20),
    _rotation(0),
    _color(115, 18, 0, 64),
    _tip(),
    _batchChanges(false),
    _changesMade(false)
{
}

BattleDialogModelEffect::BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelObject(position),
    _tokensLayer(nullptr),
    _active(true),
    _visible(true),
    _size(size),
    _rotation(rotation),
    _color(color),
    _tip(tip),
    _batchChanges(false),
    _changesMade(false)
{
}

BattleDialogModelEffect::~BattleDialogModelEffect()
{
}

QDomElement BattleDialogModelEffect::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(isExport);
    return BattleDialogModelObject::outputXML(doc, parent, targetDirectory, false);
}

void BattleDialogModelEffect::inputXML(const QDomElement &element, bool isImport)
{
    _active = static_cast<bool>(element.attribute("active", QString::number(1)).toInt());
    _visible = static_cast<bool>(element.attribute("visible", QString::number(1)).toInt());
    _size = element.attribute("size", QString::number(20)).toInt();
    _rotation = element.attribute("rotation", QString::number(0)).toDouble();
    _color = QColor(element.attribute("colorR", QString::number(115)).toInt(),
                    element.attribute("colorG", QString::number(18)).toInt(),
                    element.attribute("colorB", QString::number(0)).toInt(),
                    element.attribute("colorA", QString::number(64)).toInt());
    _tip = element.attribute("tip");

    BattleDialogModelObject::inputXML(element, isImport);
}

void BattleDialogModelEffect::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffect* otherEffect = dynamic_cast<const BattleDialogModelEffect*>(other);
    if(!otherEffect)
        return;

    _tokensLayer = otherEffect->_tokensLayer;
    _active = otherEffect->_active;
    _visible = otherEffect->_visible;
    _size = otherEffect->_size;
    _rotation = otherEffect->_rotation;
    _color = otherEffect->_color;
    _tip = otherEffect->_tip;

    BattleDialogModelObject::copyValues(other);
}

int BattleDialogModelEffect::getObjectType() const
{
    return DMHelper::CampaignType_BattleContentEffect;
}

void BattleDialogModelEffect::setLayer(LayerTokens* tokensLayer)
{
    _tokensLayer = tokensLayer;
}

LayerTokens* BattleDialogModelEffect::getLayer() const
{
    return _tokensLayer;
}

BattleDialogEffectSettings* BattleDialogModelEffect::getEffectEditor() const
{
    return new BattleDialogEffectSettings(*this);
}

void BattleDialogModelEffect::beginBatchChanges()
{
    _batchChanges = true;
    _changesMade = false;
}

void BattleDialogModelEffect::endBatchChanges()
{
    if(_batchChanges)
    {
        _batchChanges = false;
        if(_changesMade)
        {
//            emit effectChanged(this);
            emit dirty();
        }
    }
}

void BattleDialogModelEffect::applyEffectValues(QGraphicsItem& item, qreal gridScale)
{
    beginBatchChanges();
    qDebug() << "[Battle Dialog Model Effect] applying effect values for " << this << " to " << &item;
    item.setPos(getPosition());
    item.setRotation(getRotation());
    item.setToolTip(getTip());
    applyScale(item, gridScale);
    endBatchChanges();
}

void BattleDialogModelEffect::applyScale(QGraphicsItem& item, qreal gridScale)
{
    if(gridScale <= 0.0)
        return;

    item.setScale(static_cast<qreal>(getSize()) * gridScale / 500.0);
}

qreal BattleDialogModelEffect::getScale()
{
    return static_cast<qreal>(getSize()) / 500.0;
}

bool BattleDialogModelEffect::getEffectActive() const
{
    return _active;
}

void BattleDialogModelEffect::setEffectActive(bool active)
{
    if(_active != active)
    {
        _active = active;
        registerChange();
    }
}

bool BattleDialogModelEffect::getEffectVisible() const
{
    return _visible;
}

void BattleDialogModelEffect::setEffectVisible(bool visible)
{
    if(_visible != visible)
    {
        _visible = visible;
        registerChange();
    }
}

int BattleDialogModelEffect::getSize() const
{
    return _size;
}

void BattleDialogModelEffect::setSize(int size)
{
    if(_size != size)
    {
        _size = size;
        registerChange();
    }
}

int BattleDialogModelEffect::getWidth() const
{
    return -1;
}

void BattleDialogModelEffect::setWidth(int width)
{
    Q_UNUSED(width);
}

qreal BattleDialogModelEffect::getRotation() const
{
    return _rotation;
}

void BattleDialogModelEffect::setRotation(qreal rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        emit objectMoved(this);
    }
}

QColor BattleDialogModelEffect::getColor() const
{
    return _color;
}

void BattleDialogModelEffect::setColor(const QColor& color)
{
    if(_color != color)
    {
        _color = color;
        registerChange();
    }
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
    if(_tip != tip)
    {
        _tip = tip;
        registerChange();
    }
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

BattleDialogModelEffect* BattleDialogModelEffect::getFinalEffect(BattleDialogModelEffect* effect)
{
    if(!effect)
        return nullptr;

    BattleDialogModelEffect* parentEffect = qobject_cast<BattleDialogModelEffect*>(effect->parent());
    return parentEffect ? parentEffect : effect;
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
    return doc.createElement("battleeffect");
}

void BattleDialogModelEffect::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("type", getEffectType());
    element.setAttribute("active", _active);
    element.setAttribute("visible", _visible);
    element.setAttribute("size", _size);
    element.setAttribute("rotation", _rotation);
    element.setAttribute("colorR", _color.red());
    element.setAttribute("colorG", _color.green());
    element.setAttribute("colorB", _color.blue());
    element.setAttribute("colorA", _color.alpha());
    element.setAttribute("tip", _tip);

    BattleDialogModelObject::internalOutputXML(doc, element, targetDirectory, isExport);
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
    //item.setZValue(DMHelper::BattleDialog_Z_BackHighlight);
}

void BattleDialogModelEffect::registerChange()
{
    if(_batchChanges)
    {
        _changesMade = true;
    }
    else
    {
        emit effectChanged(this);
        emit dirty();
    }
}
