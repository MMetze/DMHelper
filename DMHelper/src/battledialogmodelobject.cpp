#include "battledialogmodelobject.h"
#include <QDomElement>
#include <QGraphicsItem>

BattleDialogModelObject::BattleDialogModelObject(const QPointF& position, qreal rotation, const QString& name, QObject *parent) :
    CampaignObjectBase{name, parent},
    _linkedObject(nullptr),
    _linkedId(),
    _position(position),
    _rotation(rotation)
{
}

BattleDialogModelObject::~BattleDialogModelObject()
{
}

void BattleDialogModelObject::inputXML(const QDomElement &element, bool isImport)
{
    _position = QPointF(element.attribute("positionX", QString::number(0)).toDouble(),
                        element.attribute("positionY", QString::number(0)).toDouble());
    _rotation = element.attribute("rotation", QString::number(0)).toDouble();

    if(element.hasAttribute("linkID"))
        _linkedId = QUuid(element.attribute("linkID"));

    CampaignObjectBase::inputXML(element, isImport);
}

void BattleDialogModelObject::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelObject* otherObject = dynamic_cast<const BattleDialogModelObject*>(other);
    if(!otherObject)
        return;

    _position = otherObject->_position;
    _rotation = otherObject->_rotation;

    CampaignObjectBase::copyValues(other);
}

BattleDialogModelObject* BattleDialogModelObject::getLinkedObject() const
{
    return _linkedObject;
}

QUuid BattleDialogModelObject::getLinkedID() const
{
    return _linkedId;
}

QPointF BattleDialogModelObject::getPosition() const
{
    return _position;
}

void BattleDialogModelObject::setPosition(const QPointF& position)
{
    if(_position != position)
    {
        QPointF oldPosition = _position;
        _position = position;
        emit objectMovedDelta(position - oldPosition);
        emit objectMoved(this);
        emit dirty();
    }
}

void BattleDialogModelObject::setPosition(qreal x, qreal y)
{
    setPosition(QPointF(x, y));
}

qreal BattleDialogModelObject::getRotation() const
{
    return _rotation;
}

void BattleDialogModelObject::setRotation(qreal rotation)
{
    if(_rotation != rotation)
    {
        _rotation = rotation;
        emit objectMoved(this);
        emit dirty();
    }
}

void BattleDialogModelObject::applyScale(QGraphicsItem& item, qreal gridScale)
{
    if(gridScale <= 0.0)
        return;

    item.setScale(gridScale);
}

qreal BattleDialogModelObject::getScale()
{
    return 1.0;
}

void BattleDialogModelObject::setLinkedObject(BattleDialogModelObject* linkedObject)
{
    if(_linkedObject == linkedObject)
        return;

    BattleDialogModelObject* previousLink = _linkedObject;
    _linkedObject = linkedObject;
    emit linkChanged(this, previousLink);
    emit dirty();
}

void BattleDialogModelObject::objectRemoved(BattleDialogModelObject* removedObject)
{
    if((!removedObject) || (_linkedObject != removedObject))
        return;

    setLinkedObject(nullptr);
    setLinkedObject(dynamic_cast<BattleDialogModelObject*>(removedObject->getLinkedObject()));
}

void BattleDialogModelObject::setSelected(bool isSelected)
{
    Q_UNUSED(isSelected);
}

void BattleDialogModelObject::parentMoved(const QPointF& delta)
{
    setPosition(_position + delta);
}

void BattleDialogModelObject::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());
    if(_rotation != 0.0)
        element.setAttribute("rotation", _rotation);

    if(_linkedObject)
        element.setAttribute("linkID", _linkedObject->getID().toString());

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
