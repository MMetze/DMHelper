#include "campaignobjectbase.h"
#include "campaign.h"
#include "campaignobjectfactory.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QIcon>
#include <QDebug>

// Uncomment the next line to log in detail all of the campaign item input, output and postprocessing
// #define CAMPAIGN_OBJECT_LOGGING

CampaignObjectBase::CampaignObjectBase(const QString& name, QObject *parent) :
    DMHObjectBase(parent),
    _expanded(true),
    _row(-1),
    _iconFile()
#ifdef QT_DEBUG
    , _DEBUG_NAME(name)
#endif
{
    if(!name.isEmpty())
        setObjectName(name);
}

CampaignObjectBase::~CampaignObjectBase()
{
    emit campaignObjectDestroyed(getID());
}

QDomElement CampaignObjectBase::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport)
{
#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Outputting object started: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif
    QDomElement newElement = createOutputXML(doc);
    if(!newElement.isNull())
    {
        internalOutputXML(doc, newElement, targetDirectory, isExport);

        if(!isExport)
        {
            QList<CampaignObjectBase*> childList = getChildObjects();
            for(int i = 0; i < childList.count(); ++i)
            {
                childList.at(i)->outputXML(doc, newElement, targetDirectory, isExport);
            }
        }

        parent.appendChild(newElement);
#ifdef CAMPAIGN_OBJECT_LOGGING
        qDebug() << "[CampaignBaseObject] Outputting object done: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
#endif
    }
#ifdef CAMPAIGN_OBJECT_LOGGING
    else
    {
        qDebug() << "[CampaignBaseObject] Outputting object done: " << getName() << ", type: " << getObjectType() << ", id: " << getID();
    }
#endif

    return newElement;
}

void CampaignObjectBase::inputXML(const QDomElement &element, bool isImport)
{
    DMHObjectBase::inputXML(element, isImport);

    _expanded = static_cast<bool>(element.attribute("expanded", QString::number(0)).toInt());
    _row = element.attribute("row", QString::number(-1)).toInt();

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

    _iconFile = element.attribute("base-icon");

#ifdef QT_DEBUG
    _DEBUG_NAME = objectName();
#endif

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
    qDebug() << "[CampaignBaseObject] Post-processing object started: " << element.tagName();
#endif

    QDomElement childElement = element.firstChildElement();
    while(!childElement.isNull())
    {
        //if(!belongsToObject(childElement))
        {
//            QString elTagName = childElement.tagName();
//            QString elName = childElement.attribute(QString("name"));

            QUuid baseIDValue= QUuid(childElement.attribute(QString("_baseID")));
            if(!baseIDValue.isNull())
            {
                CampaignObjectBase* childObject = searchChildrenById(baseIDValue);
                if(childObject)
                    childObject->internalPostProcessXML(childElement, isImport);
            }

            CampaignObjectBase::postProcessXML(childElement, isImport);
        }

        childElement = childElement.nextSiblingElement();
    }

#ifdef CAMPAIGN_OBJECT_LOGGING
    qDebug() << "[CampaignBaseObject] Post-processing object done: " << element.tagName();
#endif
}

void CampaignObjectBase::copyValues(const CampaignObjectBase* other)
{
    if(!other)
        return;

    setName(other->getName());
}

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

int CampaignObjectBase::getRow() const
{
    return _row;
}

bool CampaignObjectBase::isTreeVisible() const
{
    return true;
}

QIcon CampaignObjectBase::getIcon()
{
    if(!_iconFile.isEmpty())
    {
        QPixmap pixmap(_iconFile);
        if(!pixmap.isNull())
            return QIcon(pixmap.scaled(128, 128, Qt::KeepAspectRatio));
    }

    return getDefaultIcon();
}

QIcon CampaignObjectBase::getDefaultIcon()
{
    return QIcon(":/img/data/icon_contenttextencounter.png");
}

QString CampaignObjectBase::getIconFile() const
{
    return _iconFile;
}

const QList<CampaignObjectBase*> CampaignObjectBase::getChildObjects() const
{
    return findChildren<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
}

QList<CampaignObjectBase*> CampaignObjectBase::getChildObjects()
{
    return findChildren<CampaignObjectBase *>(QString(), Qt::FindDirectChildrenOnly);
}

QList<CampaignObjectBase*> CampaignObjectBase::getChildObjectsByType(int childType)
{
    QList<CampaignObjectBase*> objects;

    if(getObjectType() == childType)
        objects.append(this);

    for(CampaignObjectBase* child : getChildObjects())
    {
        if(child)
            objects.append(child->getChildObjectsByType(childType));
    }

    return objects;
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

CampaignObjectBase* CampaignObjectBase::searchDirectChildrenByName(const QString& childName)
{
    QList<CampaignObjectBase*> childList = getChildObjects();
    for(int i = 0; i < childList.count(); ++i)
    {
        if(childList.at(i)->getName() == childName)
            return childList.at(i);

        CampaignObjectBase* childResult = childList.at(i)->searchDirectChildrenByName(childName);
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

    if(object->parent() == this)
        return object->getID();

    if(object->parent())
        disconnect(object, nullptr, object->parent(), nullptr);

    object->setParent(this);

    connect(object, &CampaignObjectBase::dirty, this, &CampaignObjectBase::handleInternalDirty);
    connect(object, &CampaignObjectBase::changed, this, &CampaignObjectBase::handleInternalChange);
    handleInternalChange();
    handleInternalDirty();

    return object->getID();
}

CampaignObjectBase* CampaignObjectBase::removeObject(CampaignObjectBase* object)
{
    if(!object)
        return nullptr;

    object->setParent(nullptr);
    handleInternalChange();
    handleInternalDirty();

    return object;
}

CampaignObjectBase* CampaignObjectBase::removeObject(QUuid id)
{
    CampaignObjectBase* removed = getObjectById(id);
    if(!removed)
    {
        qDebug() << "[CampaignBaseObject] Not able to find removed object: " << id;
        return nullptr;
    }

    return removeObject(removed);
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

bool CampaignObjectBase::matchSearch(const QString& searchString, QString& result) const
{
    Q_UNUSED(result);
    return getName().contains(searchString, Qt::CaseInsensitive);
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
#ifdef QT_DEBUG
        _DEBUG_NAME = name;
#endif
        emit nameChanged(this, objectName());
        handleInternalChange();
    }
}

void CampaignObjectBase::setRow(int row)
{
    if(_row != row)
    {
        _row = row;
        handleInternalDirty();
    }
}

void CampaignObjectBase::setIconFile(const QString& iconFile)
{
    if(iconFile == _iconFile)
        return;

    _iconFile = iconFile;
    emit iconFileChanged(this);
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
    element.setAttribute("expanded", _expanded);
    element.setAttribute("row", _row);
    element.setAttribute("name", objectName());
    if(!_iconFile.isEmpty())
        element.setAttribute("base-icon", _iconFile);

    DMHObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
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
        // Integer ID support is needed for backwards compatibility to v1.5 or earlier
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
