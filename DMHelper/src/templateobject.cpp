#include "templateobject.h"
#include "templatefactory.h"
#include "globalsearch.h"
#include <QDomElement>

TemplateObject::TemplateObject(TemplateFactory* factory) :
    _factory(factory)
{
}

TemplateObject::~TemplateObject()
{
}

TemplateFactory* TemplateObject::getFactory() const
{
    return _factory;
}

void TemplateObject::setFactory(TemplateFactory* factory)
{
    _factory = factory;
}

bool TemplateObject::matchSearchString(const QString& searchString, QString& result) const
{
    if(!_factory)
        return false;

    QHash<QString, DMHAttribute> elementAttributes = _factory->getElements();
    QString searchResult;
    for(auto keyIt = elementAttributes.keyBegin(), end = elementAttributes.keyEnd(); keyIt != end; ++keyIt)
    {
        DMHAttribute attribute = elementAttributes.value(*keyIt);
        if((attribute._type == TemplateFactory::TemplateType_html) || (attribute._type == TemplateFactory::TemplateType_string))
        {
            QString value = getStringValue(*keyIt);
            if(GlobalSearch_Interface::compareStringValue(value, searchString, searchResult))
            {
                result = *keyIt + ": " + searchResult;
                return true;
            }
        }
        else if(attribute._type == TemplateFactory::TemplateType_list)
        {
            QList<QVariant> listValues = getListValue(*keyIt);
            for(const auto &value : std::as_const(listValues))
            {
                QHash<QString, QVariant> valueHash = value.toHash();
                for(auto valueKeyIt = valueHash.keyBegin(), valueEnd = valueHash.keyEnd(); valueKeyIt != valueEnd; ++valueKeyIt)
                {
                    QString value = valueHash.value(*valueKeyIt).toString();
                    if(GlobalSearch_Interface::compareStringValue(value, searchString, searchResult))
                    {
                        result = *keyIt + ": " + *valueKeyIt + ": " + searchResult;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TemplateObject::hasValue(const QString& key) const
{
    if(isAttributeSpecial(key))
        return true;

    if(valueHash()->contains(key))
        return true;

    return _factory->hasEntry(key);
}

QString TemplateObject::getValueAsString(const QString& key) const
{
    if(isAttributeSpecial(key))
        return getAttributeSpecialAsString(key);

    if((!_factory) || (!_factory->hasEntry(key)))
        return QString();

    DMHAttribute attribute = _factory->getAttribute(key);
    switch(attribute._type)
    {
    case TemplateFactory::TemplateType_string:
    case TemplateFactory::TemplateType_html:
        return getStringValue(key);
    case TemplateFactory::TemplateType_integer:
        return QString::number(getIntValue(key));
    case TemplateFactory::TemplateType_boolean:
        return getBoolValue(key) ? "true" : "false";
    case TemplateFactory::TemplateType_dice:
        return getDiceValue(key).toString();
    case TemplateFactory::TemplateType_resource:
    {
        ResourcePair pair = getResourceValue(key);
        return QString::number(pair.first) + QString(",") + QString::number(pair.second);
    }
    default:
        qDebug() << "[Characterv2] WARNING: Trying to read value for unexpected key: " << key << " with type " << attribute._type;
        return QString();
    }
}

QString TemplateObject::getStringValue(const QString& key) const
{
    if(isAttributeSpecial(key))
    {
        return getAttributeSpecialAsString(key);
    }
    else
    {
        const QHash<QString, QVariant>* hash = valueHash();
        return hash->contains(key) ? hash->value(key, QString()).toString() : _factory->getAttribute(key)._default;
    }
}

int TemplateObject::getIntValue(const QString& key) const
{
    if(isAttributeSpecial(key))
    {
        return getAttributeSpecial(key).toInt();
    }
    else
    {
        const QHash<QString, QVariant>* hash = valueHash();
        return hash->contains(key) ? hash->value(key, 0).toInt() : _factory->getAttribute(key)._default.toInt();
    }
}

bool TemplateObject::getBoolValue(const QString& key) const
{
    if(isAttributeSpecial(key))
    {
        return getAttributeSpecial(key).toBool();
    }
    else
    {
        const QHash<QString, QVariant>* hash = valueHash();
        return hash->contains(key) ? hash->value(key, false).toBool() : static_cast<bool>(_factory->getAttribute(key)._default.toInt());
    }
}

Dice TemplateObject::getDiceValue(const QString& key) const
{
    if(isAttributeSpecial(key))
    {
        return getAttributeSpecial(key).value<Dice>();
    }
    else
    {
        const QHash<QString, QVariant>* hash = valueHash();
        return hash->contains(key) ? hash->value(key, QVariant()).value<Dice>() : Dice(_factory->getAttribute(key)._default);
    }
}

ResourcePair TemplateObject::getResourceValue(const QString& key) const
{
    if(isAttributeSpecial(key))
    {
        return getAttributeSpecial(key).value<ResourcePair>();
    }
    else
    {
        const QHash<QString, QVariant>* hash = valueHash();
        if(hash->contains(key))
        {
            return hash->value(key, QVariant()).value<ResourcePair>();
        }
        else
        {
            QString value = _factory->getAttribute(key)._default;
            QStringList resourceString = value.split(QString(","));
            if(resourceString.size() == 2)
                return ResourcePair(resourceString.at(0).toInt(), resourceString.at(1).toInt());
            else
                return ResourcePair();
        }
    }
}

QList<QVariant> TemplateObject::getListValue(const QString& key) const
{
    return valueHash()->value(key).toList();
}

void TemplateObject::setValue(const QString& key, const QVariant& value)
{
    if(valueHash()->value(key) == value)
        return;

    valueHash()->insert(key, value);
    declareDirty();
}

void TemplateObject::setValue(const QString& key, const QString& value)
{
    if(isAttributeSpecial(key))
        setAttributeSpecial(key, value);

    if((!valueHash()->contains(key)) && (!_factory->hasEntry(key)))
    {
        qDebug() << "[Characterv2] Attempting to set the value \"" << value << "\" for the unknown key " << key;
        return;
    }

    DMHAttribute attribute = _factory->getAttribute(key);
    switch(attribute._type)
    {
    case TemplateFactory::TemplateType_string:
    case TemplateFactory::TemplateType_html:
        setValue(key, QVariant(value));
        break;
    case TemplateFactory::TemplateType_integer:
        setValue(key, QVariant(value.toInt()));
        break;
    case TemplateFactory::TemplateType_boolean:
        setValue(key, QVariant(static_cast<bool>(value.toInt())));
        break;
    case TemplateFactory::TemplateType_dice:
    {
        QVariant newValue;
        newValue.setValue(Dice(value));
        setValue(key, newValue);
        break;
    }
    case TemplateFactory::TemplateType_resource:
    {
        QStringList resourceString = value.split(QString(","));
        if(resourceString.size() == 2)
        {
            QVariant newValue;
            newValue.setValue(ResourcePair(resourceString.at(0).toInt(), resourceString.at(1).toInt()));
            setValue(key, newValue);
        }
        else
            qDebug() << "[Characterv2] WARNING: Resource attribute missing values: " << key << " with type " << attribute._type << ": " << value;
        break;
    }
    default:
        qDebug() << "[Characterv2] WARNING: Setting value for unexpected key: " << key << " with type " << attribute._type << " and value " << value;
        break;
    }
}

void TemplateObject::setStringValue(const QString& key, const QString& value)
{
    setValue(key, QVariant(value));
}

void TemplateObject::setIntValue(const QString& key, int value)
{
    setValue(key, QVariant(value));
}

void TemplateObject::setBoolValue(const QString& key, bool value)
{
    setValue(key, QVariant(value));
}

void TemplateObject::setDiceValue(const QString& key, const Dice& value)
{
    QVariant newValue;
    newValue.setValue(value);
    setValue(key, newValue);
}

void TemplateObject::setResourceValue(const QString& key, const ResourcePair& value)
{
    QVariant newValue;
    newValue.setValue(value);
    setValue(key, newValue);
}

void TemplateObject::setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue)
{
    QList<QVariant> list = getListValue(key);

    if((index < 0) || (index >= list.size()))
        return;

    QHash<QString, QVariant> listEntryValues = list.at(index).toHash();
    listEntryValues.insert(listEntryKey, listEntryValue);
    list.replace(index, listEntryValues);
    setValue(key, QVariant(list));
}

void TemplateObject::setListValue(const QString& key, int index, const QString& listEntryKey, const QString& listEntryValue)
{
    QHash<QString, DMHAttribute> listAttributes = _factory->getElementList(key);
    DMHAttribute attribute = listAttributes.value(listEntryKey);
    setListValue(key, index, listEntryKey, TemplateFactory::convertStringToVariant(listEntryValue, attribute._type));
}

QHash<QString, QVariant> TemplateObject::createListEntry(const QString& key, int index)
{
    if(!_factory->hasElementList(key))
    {
        qDebug() << "[Characterv2] WARNING: Request to create list entry for an invalid list key: " << key;
        return QHash<QString, QVariant>();
    }

    QHash<QString, QVariant> newEntryValues;

    // Iterate through the list and create the individual attributes
    QHash<QString, DMHAttribute> listAttributes = _factory->getElementList(key);
    for(auto keyIt = listAttributes.keyBegin(), end = listAttributes.keyEnd(); keyIt != end; ++keyIt)
    {
        DMHAttribute attribute = listAttributes.value(*keyIt);
        newEntryValues.insert(*keyIt, TemplateFactory::convertStringToVariant(attribute._default, attribute._type));
    }

    insertListEntry(key, index, newEntryValues);
    return newEntryValues;
}

void TemplateObject::insertListEntry(const QString& key, int index, QHash<QString, QVariant> listEntryValues)
{
    if(listEntryValues.isEmpty())
        return;

    QList<QVariant> list = getListValue(key);

    if((index < 0) || (index > list.size()))
    {
        qDebug() << "[Characterv2] WARNING: Request to insert invalid index " << index << " into list " << key;
        return;
    }

    list.insert(index, QVariant(listEntryValues));
    setValue(key, QVariant(list));
}

void TemplateObject::prependListEntry(const QString& key, QHash<QString, QVariant> listEntryValues)
{
    insertListEntry(key, 0, listEntryValues);
}

void TemplateObject::appendListEntry(const QString& key, QHash<QString, QVariant> listEntryValues)
{
    insertListEntry(key, getListValue(key).size(), listEntryValues);
}

void TemplateObject::removeListEntry(const QString& key, int index)
{
    QList<QVariant> list = getListValue(key);
    if(list.isEmpty())
    {
        qDebug() << "[Characterv2] WARNING: Request to remove item from unknown list " << key;
        return;
    }

    if((index < 0) || (index >= list.size()))
    {
        qDebug() << "[Characterv2] WARNING: Request to move invalid index " << index << " from list " << key;
        return;
    }

    list.removeAt(index);
    setValue(key, QVariant(list));
}


void TemplateObject::readXMLValues(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if((!_factory) || (element.isNull()))
        return;

    // Iterate through expected attributes
    QHash<QString, DMHAttribute> attributeHash = _factory->getAttributes();
    for(auto keyIt = attributeHash.keyBegin(), end = attributeHash.keyEnd(); keyIt != end; ++keyIt)
    {
        if(!isAttributeSpecial(*keyIt))
        {
            QVariant attributeValue = readAttributeValue(_factory->getAttribute(*keyIt), element, *keyIt);
            if(!attributeValue.isNull())
                valueHash()->insert(*keyIt, attributeValue);
        }
    }

    // Child elements
    QDomElement childElement = element.firstChildElement();
    while(!childElement.isNull())
    {
        QString tagName = childElement.tagName();
        if(_factory->hasElement(tagName))
        {
            DMHAttribute attribute = _factory->getElement(tagName);
            if(attribute._type == TemplateFactory::TemplateType_html)
            {
                QDomNode dataChildNode = childElement.firstChild();
                if((!dataChildNode.isNull()) && (dataChildNode.isCDATASection()))
                {
                    QDomCDATASection dataSection = dataChildNode.toCDATASection();
                    valueHash()->insert(tagName, dataSection.data());
                }
                else
                {
                    qDebug() << "[Characterv2] WARNING: HTML child element missing a data node: " << tagName << " with type " << attribute._type;
                }
            }
            else
            {
                qDebug() << "[Characterv2] WARNING: Unexpected child element: " << tagName << " with type " << attribute._type;
            }
        }
        else if(_factory->hasElementList(tagName))
        {
            QList<QVariant> listValues;

            // Iterate through the list and create the individual attributes
            QHash<QString, DMHAttribute> listAttributes = _factory->getElementList(tagName);
            QDomElement listEntry = childElement.firstChildElement();
            while(!listEntry.isNull())
            {
                QHash<QString, QVariant> listEntryValues;
                for(auto keyIt = listAttributes.keyBegin(), end = listAttributes.keyEnd(); keyIt != end; ++keyIt)
                {
                    QVariant attributeValue = readAttributeValue(listAttributes.value(*keyIt), listEntry, *keyIt);
                    if(!attributeValue.isNull())
                        listEntryValues.insert(*keyIt, attributeValue);
                }

                // Add the list entry to the list
                listValues.append(listEntryValues);

                listEntry = listEntry.nextSiblingElement();
            }

            // Add the list entry to the main list
            valueHash()->insert(tagName, listValues);
        }
#ifdef QT_DEBUG
        else
        {
            qDebug() << "[TemplateObject] WARNING: Unknown value type: " << tagName;
        }
#endif

        childElement = childElement.nextSiblingElement();
    }

    handleOldXMLs(element);
}

void TemplateObject::writeXMLValues(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const
{
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    if(!_factory)
        return;

    QList<QString> valueKeys = valueHash()->keys();
    for (const auto &key : std::as_const(valueKeys))
    {
        if(_factory->hasAttribute(key))
            writeAttributeValue(_factory->getAttribute(key), element, key, valueHash()->value(key));
        else if(_factory->hasElement(key))
            writeElementValue(doc, element, key, valueHash()->value(key));
        else if(_factory->hasElementList(key))
            writeElementListValue(doc, element, key, valueHash()->value(key));
    }
}

void TemplateObject::handleOldXMLs(const QDomElement& element)
{
    Q_UNUSED(element);
}

bool TemplateObject::isAttributeSpecial(const QString& attribute) const
{
    Q_UNUSED(attribute);
    return false;
}

QVariant TemplateObject::getAttributeSpecial(const QString& attribute) const
{
    Q_UNUSED(attribute);
    return QVariant();
}

QString TemplateObject::getAttributeSpecialAsString(const QString& attribute) const
{
    Q_UNUSED(attribute);
    return QString();
}

void TemplateObject::setAttributeSpecial(const QString& key, const QString& value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
}

QVariant TemplateObject::readAttributeValue(const DMHAttribute& attribute, const QDomElement& element, const QString& name)
{
    return TemplateFactory::convertStringToVariant(element.attribute(name, attribute._default), attribute._type);
}

void TemplateObject::writeAttributeValue(const DMHAttribute& attribute, QDomElement& element, const QString& key, const QVariant& value) const
{
    if((key.isEmpty()) || (value.isNull()))
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an invalid/empty attribute. Key: " << key << ", value: " << value;
        return;
    }

    element.setAttribute(key, TemplateFactory::convertVariantToString(value, attribute._type));
}

void TemplateObject::writeElementValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value) const
{
    if((key.isEmpty()) || (value.isNull()))
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an invalid/empty element. Key: " << key << ", value: " << value;
        return;
    }

    DMHAttribute attribute = _factory->getElement(key);
    if(attribute._type != TemplateFactory::TemplateType_html)
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an unexpected element: " << key << " with type " << attribute._type;
        return;
    }

    if(!value.canConvert<QString>())
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an invalid element value: " << key << " with type " << attribute._type;
        return;
    }

    QString stringValue = value.toString();
    if(stringValue.isEmpty())
        return; // Nothing to write

    QDomElement childElement = doc.createElement(key);
    QDomCDATASection childData = doc.createCDATASection(stringValue);
    childElement.appendChild(childData);
    element.appendChild(childElement);
}

void TemplateObject::writeElementListValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value) const
{
    if((key.isEmpty()) || (value.isNull()))
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an invalid/empty element list. Key: " << key << ", value: " << value;
        return;
    }

    if(!_factory->hasElementList(key))
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an unexpected element list: " << key;
        return;
    }

    QList<QVariant> listValues = value.toList();
    if(listValues.isEmpty())
    {
        qDebug() << "[Characterv2] WARNING: Trying to write an empty element list: " << key;
        return;
    }

    QHash<QString, DMHAttribute> listAttributes = _factory->getElementList(key);
    QDomElement listElement = doc.createElement(key);
    for(const auto &value : std::as_const(listValues))
    {
        QDomElement childElement = doc.createElement(QString("entry"));

        QHash<QString, QVariant> valueHash = value.toHash();
        QList<QString> hashKeys = valueHash.keys();
        for(const auto &hashKey : std::as_const(hashKeys))
        {
            writeAttributeValue(listAttributes.value(hashKey), childElement, hashKey, valueHash.value(hashKey));
        }

        listElement.appendChild(childElement);
    }
    element.appendChild(listElement);
}
