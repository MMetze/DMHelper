#include "battledialogmodelobject.h"
#include <QDomElement>
#include <QGraphicsItem>

BattleDialogModelObject::BattleDialogModelObject(const QPointF& position, const QString& name, QObject *parent) :
    CampaignObjectBase{name, parent},
    _linkedObject(nullptr),
    _position(position)
{
}

BattleDialogModelObject::~BattleDialogModelObject()
{
}

void BattleDialogModelObject::inputXML(const QDomElement &element, bool isImport)
{
    _position = QPointF(element.attribute("positionX", QString::number(0)).toDouble(),
                        element.attribute("positionY", QString::number(0)).toDouble());

    CampaignObjectBase::inputXML(element, isImport);
}

void BattleDialogModelObject::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelObject* otherObject = dynamic_cast<const BattleDialogModelObject*>(other);
    if(!otherObject)
        return;

    _position = otherObject->_position;

    CampaignObjectBase::copyValues(other);
}

BattleDialogModelObject* BattleDialogModelObject::getLinkedObject() const
{
    return _linkedObject;
}

QPointF BattleDialogModelObject::getPosition() const
{
    return _position;
}

void BattleDialogModelObject::setPosition(const QPointF& position)
{
    if(_position != position)
    {
        _position = position;
        emit objectMoved(this);
    }
}

void BattleDialogModelObject::setPosition(qreal x, qreal y)
{
    setPosition(QPointF(x, y));
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

void BattleDialogModelObject::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("positionX", _position.x());
    element.setAttribute("positionY", _position.y());

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
}
