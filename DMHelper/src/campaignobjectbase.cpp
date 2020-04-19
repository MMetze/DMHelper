#include "campaignobjectbase.h"
#include "campaign.h"
#include "campaignobjectfactory.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#define CAMPAIGN_OBJECT_LOGGING

CampaignObjectBase::CampaignObjectBase(const QString& name, QObject *parent) :
    DMHObjectBase(parent),
    _expanded(true)
{
    if(!name.isEmpty())
        setObjectName(name);
}

CampaignObjectBase::~CampaignObjectBase()
{
}

QDomElement CampaignObjectBase::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Outputting object started: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif
    QDomElement newElement = createOutputXML(doc);
    internalOutputXML(doc, newElement, targetDirectory, isExport);

    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        childList.at(i)->outputXML(doc, newElement, targetDirectory, isExport);
    }

    parent.appendChild(newElement);
#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Outputting object done: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif

    return newElement;
}

void CampaignObjectBase::inputXML(const QDomElement &element, bool isImport)
{
    DMHObjectBase::inputXML(element, isImport);

    _expanded = static_cast<bool>(element.attribute("expanded",QString::number(0)).toInt());
    QString importName = element.attribute("name");
    if(importName.isEmpty())
    {
        if(getName().isEmpty())
            setObjectName(QString("unknown"));
    }
    else
    {
        setObjectName(importName);
    }

#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Inputting object started: " << getName() << ", id: " << getID();
#endif

    if(getID().isNull())
    {
        setID(findUuid(getIntID()));
    }

    QDomElement contentElement = element.firstChildElement();
    while(!contentElement.isNull())
    {
        if(!belongsToObject(contentElement))
        {
            CampaignObjectBase* newObject = CampaignObjectFactory::createObject(contentElement, isImport);
            if(newObject)
                addObject(newObject);
        }
        contentElement = contentElement.nextSiblingElement();
    }

#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Inputting object done: " << getName() << ", id: " << getID();
#endif
}

void CampaignObjectBase::postProcessXML(const QDomElement &element, bool isImport)
{
#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Post-processing object started: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif

    QDomElement childElement = element.firstChildElement();
    while(!childElement.isNull())
    {
//        if(!belongsToObject(childElement))
        {
            QString elTagName = childElement.tagName();
            QString elName = childElement.attribute(QString("name"));
            int chCount = childElement.childNodes().count();

            CampaignObjectBase* childObject = searchChildrenById(QUuid(childElement.attribute(QString("_baseID"))));
            if(childObject)
                childObject->internalPostProcessXML(childElement, isImport);

            postProcessXML(childElement, isImport);
        }

        childElement = childElement.nextSiblingElement();
    }

#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Post-processing object done: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif
}

/*
void CampaignObjectBase::resolveReferences()
{
}
*/

int CampaignObjectBase::getObjectType() const
{
    return DMHelper::CampaignType_Base;
}

bool CampaignObjectBase::getExpanded() const
{
    return _expanded;
}

QString CampaignObjectBase::getName() const
{
    return objectName();
}

const QList<CampaignObjectBase*> CampaignObjectBase::getChildObjects() const
{
    return findChildren<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
}

QList<CampaignObjectBase*> CampaignObjectBase::getChildObjects()
{
    return findChildren<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
}

CampaignObjectBase* CampaignObjectBase::getChildById(QUuid id)
{
    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        if(childList.at(i)->getID() == id)
            return childList.at(i);
    }

    return nullptr;
}

CampaignObjectBase* CampaignObjectBase::searchChildrenById(QUuid id)
{
    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        if(childList.at(i)->getID() == id)
            return childList.at(i);

        CampaignObjectBase* childResult = childList.at(i)->searchChildrenById(id);
        if(childResult != nullptr)
            return childResult;
    }

    return nullptr;
}

const CampaignObjectBase* CampaignObjectBase::getParentByType(int parentType) const
{
    const CampaignObjectBase* parentObject = qobject_cast<const CampaignObjectBase*>(parent());
    if(!parentObject)
        return nullptr;

    if(parentObject->getObjectType() == parentType)
        return parentObject;
    else
        return parentObject->getParentByType(parentType);
}

CampaignObjectBase* CampaignObjectBase::getParentByType(int parentType)
{
    CampaignObjectBase* parentObject = qobject_cast<CampaignObjectBase*>(parent());
    if(!parentObject)
        return nullptr;

    if(parentObject->getObjectType() == parentType)
        return parentObject;
    else
        return parentObject->getParentByType(parentType);
}

const CampaignObjectBase* CampaignObjectBase::getParentById(const QUuid& id) const
{
    const CampaignObjectBase* parentObject = qobject_cast<const CampaignObjectBase*>(parent());
    if(!parentObject)
        return nullptr;

    if(parentObject->getID() == id)
        return parentObject;
    else
        return parentObject->getParentById(id);
}

CampaignObjectBase* CampaignObjectBase::getParentById(const QUuid& id)
{
    CampaignObjectBase* parentObject = qobject_cast<CampaignObjectBase*>(parent());
    if(!parentObject)
        return nullptr;

    if(parentObject->getID() == id)
        return parentObject;
    else
        return parentObject->getParentById(id);
}

QUuid CampaignObjectBase::addObject(CampaignObjectBase* object)
{
    if(!object)
        return QUuid();

    if(parent())
        disconnect(object, nullptr, parent(), nullptr);

    object->setParent(this);

    connect(object, &CampaignObjectBase::dirty, this, &CampaignObjectBase::handleInternalDirty);
    connect(object, &CampaignObjectBase::changed, this, &CampaignObjectBase::handleInternalChange);
    handleInternalChange();
    handleInternalDirty();

    return object->getID();
}

CampaignObjectBase* CampaignObjectBase::removeObject(QUuid id)
{
    CampaignObjectBase* removed = getObjectById(id);
    if(!removed)
    {
        qDebug() << "[CampaignBaseObject] Not able to find removed object: " << id;
        return nullptr;
    }

    removed->setParent(nullptr);
    handleInternalChange();
    handleInternalDirty();

    return removed;
}

CampaignObjectBase* CampaignObjectBase::getObjectById(QUuid id)
{
    if(getID() == id)
        return this;

    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        CampaignObjectBase* childObject = childList.at(i)->getObjectById(id);
        if(childObject)
            return childObject;
    }

    return nullptr;
}

const CampaignObjectBase* CampaignObjectBase::getObjectById(QUuid id) const
{
    if(getID() == id)
        return this;

    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        CampaignObjectBase* childObject = childList.at(i)->getObjectById(id);
        if(childObject)
            return childObject;
    }

    return nullptr;
}

void CampaignObjectBase::setExpanded(bool expanded)
{
    if(_expanded != expanded)
    {
        _expanded = expanded;
        emit expandedChanged(_expanded);
        handleInternalDirty();
    }
}

void CampaignObjectBase::setName(const QString& name)
{
    if(objectName() != name)
    {
        setObjectName(name);
        //emit nameChanged(objectName());
        //handleInternalDirty();
        handleInternalChange();
    }
}

void CampaignObjectBase::handleInternalChange()
{
    emit changed();
    emit dirty();
}

void CampaignObjectBase::handleInternalDirty()
{
    emit dirty();
}

void CampaignObjectBase::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(doc);
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    element.setAttribute("expanded", getExpanded());
    element.setAttribute("name", getName());
}

bool CampaignObjectBase::belongsToObject(QDomElement& element)
{
    Q_UNUSED(element);
    return false;
}

void CampaignObjectBase::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);
    Q_UNUSED(isImport);
}

QUuid CampaignObjectBase::parseIdString(QString idString, int* intId, bool isLocal)
{
    QUuid result(idString);
    if(result.isNull())
    {
        bool ok = false;
        int tempIntId = idString.toInt(&ok);
        if(ok)
        {
            if(intId)
                *intId = tempIntId;

            if(!isLocal)
                result = findUuid(tempIntId);
        }
    }

    return result;
}

QUuid CampaignObjectBase::findUuid(int intId) const
{
    if(intId == DMH_GLOBAL_INVALID_ID)
        return QUuid();

    const Campaign* campaign = dynamic_cast<const Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
        return QUuid();

    QUuid result = campaign->findChildUuid(intId);
    if(result.isNull())
        qDebug() << "[Campaign] WARNING: unable to find matching object for Integer ID " << intId;

    return result;
}

QUuid CampaignObjectBase::findChildUuid(int intId) const
{
    if(getIntID() == intId)
        return getID();

    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        QUuid result = childList.at(i)->findChildUuid(intId);
        if(!result.isNull())
            return result;
    }

    return QUuid();
}
