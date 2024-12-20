#include "templatefactory.h"
#include "templateobject.h"
#include "dice.h"
#include "combatant.h"
#include "rulefactory.h"
#include <QVariant>
#include <QCoreApplication>
#include <QDomElement>
#include <QMessageBox>
#include <QDebug>

const char* TemplateFactory::TEMPLATE_PROPERTY = "dmhValue";
const char* TemplateFactory::TEMPLATE_WIDGET = "dmhWidget";

const char* TemplateFactory::TEMPLATEVALUES[TEMPLATETYPE_COUNT] =
    {
        "dmh:template",     // TemplateType_template
        "dmh:attribute",    // TemplateType_attribute
        "dmh:element",      // TemplateType_element
        "dmh:string",       // TemplateType_string
        "dmh:integer",      // TemplateType_integer
        "dmh:boolean",      // TemplateType_boolean
        "dmh:resource",     // TemplateType_resource
        "dmh:dice",         // TemplateType_dice
        "dmh:html",         // TemplateType_html
        "dmh:list",         // TemplateType_list
};

TemplateFactory::TemplateFactory(QObject *parent) :
    ObjectFactory{parent},
    _attributes(),
    _elements(),
    _elementLists()
{
}

QVariant TemplateFactory::convertStringToVariant(const QString& value, TemplateType type)
{
    switch(type)
    {
    case TemplateFactory::TemplateType_string:
        return QVariant(value);
    case TemplateFactory::TemplateType_integer:
        return QVariant(value.toInt());
    case TemplateFactory::TemplateType_boolean:
        return QVariant(value.toInt());
    case TemplateFactory::TemplateType_dice:
    {
        QVariant diceResult;
        diceResult.setValue(Dice(value));
        return diceResult;
    }
    case TemplateFactory::TemplateType_resource:
    {
        QStringList resourceList = value.split(",");
        if(resourceList.size() != 2)
        {
            qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to a resource pair, but it is not in the correct format";
            return QVariant();
        }

        QVariant resourceResult;
        resourceResult.setValue(ResourcePair(resourceList.at(0).toInt(), resourceList.at(1).toInt()));
        return resourceResult;
    }
    default:
        qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to the unexpected attribute type: " << type;
        return QVariant();
    }
}

QString TemplateFactory::convertVariantToString(const QVariant& value, TemplateType type)
{
    switch(type)
    {
    case TemplateFactory::TemplateType_string:
        return value.toString();
    case TemplateFactory::TemplateType_integer:
        return QString::number(value.toInt());
    case TemplateFactory::TemplateType_boolean:
        return QString::number(value.toInt());
    case TemplateFactory::TemplateType_dice:
        return value.value<Dice>().toString();
    case TemplateFactory::TemplateType_resource:
    {
        ResourcePair resourcePair = value.value<ResourcePair>();
        return QString::number(resourcePair.first) + QString(",") + QString::number(resourcePair.second);
    }
    default:
        qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to the unexpected attribute type: " << type;
        return QString();
    }
}

bool TemplateFactory::isEmpty() const
{
    return _attributes.isEmpty() && _elements.isEmpty() && _elementLists.isEmpty();
}

bool TemplateFactory::hasAttribute(const QString& name) const
{
    return _attributes.contains(name);
}

DMHAttribute TemplateFactory::getAttribute(const QString& name) const
{
    return _attributes.value(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getAttributes() const
{
    return _attributes;
}

bool TemplateFactory::hasElement(const QString& name) const
{
    return _elements.contains(name);
}

DMHAttribute TemplateFactory::getElement(const QString& name) const
{
    return _elements.value(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getElements() const
{
    return _elements;
}

bool TemplateFactory::hasElementList(const QString& name) const
{
    return _elementLists.contains(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getElementList(const QString& name) const
{
    return _elementLists.value(name);
}

QHash<QString, QHash<QString, DMHAttribute>> TemplateFactory::getElementLists() const
{
    return _elementLists;
}

bool TemplateFactory::hasEntry(const QString& name) const
{
    return hasAttribute(name) || hasElement(name) || hasElementList(name);
}

TemplateObject* TemplateFactory::setDefaultValues(TemplateObject* object)
{
    if(!object)
        return nullptr;

    QHash<QString, DMHAttribute> attributes = getAttributes();
    for(auto it = attributes.begin(); it != attributes.end(); ++it)
    {
        object->setValue(it.key(), it->_default);
    }

    QHash<QString, DMHAttribute> elements = getElements();
    for(auto it = elements.begin(); it != elements.end(); ++it)
    {
        object->setValue(it.key(), it->_default);
    }

    return object;
}

void TemplateFactory::loadTemplate(const QString& templateFile)
{
    if(!RuleFactory::Instance())
    {
        qDebug() << "[TemplateFactory] ERROR: No rule factory exists, cannot load the template file: " << templateFile;
        return;
    }

    // Try our best to load the given template file
    QString appFile;
    if(QFileInfo(templateFile).isRelative())
    {
        QDir relativeDir = RuleFactory::Instance()->getRulesetDir();
        appFile = relativeDir.absoluteFilePath(templateFile);
        if(!QFileInfo::exists(appFile))
        {
#ifdef Q_OS_MAC
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            fileDirPath.cdUp();
            appFile = fileDirPath.path() + QString("/Resources/") + templateFile;
#else
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            appFile = fileDirPath.path() + QString("/resources/") + templateFile;
#endif
            if(!QFileInfo::exists(appFile))
            {
                qDebug() << "[TemplateFactory] ERROR: Relative Template File not found: " << templateFile;
                return;
            }
        }
    }
    else
    {
        appFile = templateFile;
        if(!QFileInfo::exists(appFile))
        {
            qDebug() << "[TemplateFactory] ERROR: Absolute Template File not found: " << templateFile;
            return;
        }
    }

    QDomDocument doc;
    QFile file(appFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[TemplateFactory] Template file open failed: " << appFile;
        QMessageBox::critical(nullptr, QString("Template file open failed"), QString("Unable to open the template file: ") + appFile);
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[TemplateFactory] Error reading template XML content. The XML is probably not valid: " << appFile;
        qDebug() << errMsg << errRow << errColumn;
        QMessageBox::critical(nullptr, QString("Template invalid"), QString("Unable to read the template: ") + appFile + QString(", the XML is invalid"));
        return;
    }

    _attributes.clear();
    _elements.clear();
    _elementLists.clear();

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != TEMPLATEVALUES[TemplateType_template]))
    {
        qDebug() << "[TemplateFactory] Ttemplate missing root item: " << appFile;
        QMessageBox::critical(nullptr, QString("Template file invalid"), QString("Unable to read the template: ") + appFile + QString(", the XML does not have the expected root item."));
        return;
    }

    QDomElement element = root.firstChildElement();
    while(!element.isNull())
    {
        QString elementName = element.attribute("name");
        QString elementType = element.attribute("type");
        if(element.tagName() == TEMPLATEVALUES[TemplateType_attribute])
        {
            if(elementType == TEMPLATEVALUES[TemplateType_string])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_string, element.attribute("default")));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_integer])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_integer, element.attribute("default", QString("0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_boolean])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_boolean, element.attribute("default", QString("0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_resource])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_resource, element.attribute("default", QString("0,0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_dice])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_dice, element.attribute("default")));
            }
        }
        else if(element.tagName() == TEMPLATEVALUES[TemplateType_element])
        {
            if(elementType == TEMPLATEVALUES[TemplateType_html])
            {
                _elements.insert(elementName, DMHAttribute(TemplateType_html, element.attribute("default")));
            }
            else if(elementType == TEMPLATEVALUES[TemplateType_list])
            {
                QHash<QString, DMHAttribute> newHash;
                QDomElement listElement = element.firstChildElement();
                while(!listElement.isNull())
                {
                    QString listElementName = listElement.attribute("name");
                    QString listElementType = listElement.attribute("type");
                    if(listElementType == TEMPLATEVALUES[TemplateType_string])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_string, listElement.attribute("default")));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_integer])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_integer, listElement.attribute("default", QString("0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_boolean])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_boolean, listElement.attribute("default", QString("0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_resource])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_resource, listElement.attribute("default", QString("0,0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_dice])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_dice, listElement.attribute("default")));
                    }

                    listElement = listElement.nextSiblingElement();
                }

                _elementLists.insert(elementName, newHash);
            }
        }

        element = element.nextSiblingElement();
    }

    // Validate that each entry has a name attribute
    if(!hasAttribute(QString("name")))
        qDebug() << "[TemplateFactory] ERROR: Template has no 'name' attribute";
}
