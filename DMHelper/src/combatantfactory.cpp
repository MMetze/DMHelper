#include "combatantfactory.h"
#include "characterv2.h"
#include "monster.h"
#include "dmconstants.h"
#include "bestiary.h"
#include "combatantreference.h"
#include <QDomElement>
#include <QString>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>

CombatantFactory* CombatantFactory::_instance = nullptr;

const char* CombatantFactory::TEMPLATE_PROPERTY = "dmhValue";
const char* CombatantFactory::TEMPLATE_WIDGET = "dmhWidget";

const char* CombatantFactory::TEMPLATEVALUES[TEMPLATETYPE_COUNT] =
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


CombatantFactory::CombatantFactory(QObject *parent) :
    ObjectFactory(parent),
    _attributes(),
    _elements(),
    _elementLists()
{
}

CombatantFactory* CombatantFactory::Instance()
{
    if(!_instance)
    {
        qDebug() << "[CombatantFactory] Initializing Combatant Factory";
        _instance = new CombatantFactory();
        _instance->loadCharacterTemplate();
    }

    return _instance;
}

void CombatantFactory::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

Combatant* CombatantFactory::createCombatant(int combatantType, const QDomElement& element, bool isImport, QObject *parent)
{
    Combatant* combatant = nullptr;

    switch(combatantType)
    {
        case DMHelper::CombatantType_Monster:
            combatant = Bestiary::Instance()->createMonster(element, isImport);
            break;
        case DMHelper::CombatantType_Reference:
            combatant = new CombatantReference(QString(), parent);
            break;
        case DMHelper::CombatantType_Base:
        case DMHelper::CombatantType_Character:
            //combatant = new Characterv2(parent);
            //break;
        default:
            qDebug() << "[CombatantFactory] ERROR: unexpected creation of battle combatant type: " << combatantType;
            break;
   }

    if(combatant)
        combatant->inputXML(element, isImport);

    return combatant;
}

CampaignObjectBase* CombatantFactory::createObject(int objectType, int subType, const QString& objectName, bool isImport)
{
    Q_UNUSED(isImport);

    if(objectType != DMHelper::CampaignType_Combatant)
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected creation of non-combatant object type: " << objectType << ", subType: " << subType;
        return nullptr;
    }

    switch(subType)
    {
        case DMHelper::CombatantType_Character:
            return new Characterv2(objectName);
        case DMHelper::CombatantType_Reference:
            return new CombatantReference();
        case DMHelper::CombatantType_Base:
        case DMHelper::CombatantType_Monster:
        default:
            return nullptr;
    }
}

CampaignObjectBase* CombatantFactory::createObject(const QDomElement& element, bool isImport)
{
    Q_UNUSED(isImport);

    if(element.tagName() != QString("combatant"))
        return nullptr;

    bool ok = false;
    int combatantType = element.attribute("type").toInt(&ok);

    if((!ok) || (combatantType != DMHelper::CombatantType_Character))
    {
        qDebug() << "[CombatantFactory] ERROR: unexpected combatant type found reading element: " << combatantType << ", read success: " << ok;
        return nullptr;
    }

    return new Characterv2();
}

bool CombatantFactory::hasAttribute(const QString& name) const
{
    return _attributes.contains(name);
}

DMHAttribute CombatantFactory::getAttribute(const QString& name) const
{
    return _attributes.value(name);
}

QHash<QString, DMHAttribute> CombatantFactory::getAttributes() const
{
    return _attributes;
}

bool CombatantFactory::hasElement(const QString& name) const
{
    return _elements.contains(name);
}

DMHAttribute CombatantFactory::getElement(const QString& name) const
{
    return _elements.value(name);
}

QHash<QString, DMHAttribute> CombatantFactory::getElements() const
{
    return _elements;
}

bool CombatantFactory::hasElementList(const QString& name) const
{
    return _elementLists.contains(name);
}

QHash<QString, DMHAttribute> CombatantFactory::getElementList(const QString& name) const
{
    return _elementLists.value(name);
}

QHash<QString, QHash<QString, DMHAttribute>> CombatantFactory::getElementLists() const
{
    return _elementLists;
}

void CombatantFactory::loadCharacterTemplate()
{
    QString defaultFilename("character.xml");
#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString appFile = fileDirPath.path() + QString("/Resources/") + defaultFilename;
#else
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    QString appFile = fileDirPath.path() + QString("/resources/") + defaultFilename;
#endif

    if(!QFileInfo::exists(appFile))
    {
        qDebug() << "[CombatantFactory] ERROR: Character Template File not found: " << appFile;
        return;
    }

    QDomDocument doc;
    QFile file(appFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[CombatantFactory] Character Template file open failed: " << appFile;
        QMessageBox::critical(nullptr, QString("Character Template file open failed"), QString("Unable to open the character template: ") + appFile);
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
        qDebug() << "[CombatantFactory] Error reading character template XML content. The XML is probably not valid: " << appFile;
        qDebug() << errMsg << errRow << errColumn;
        QMessageBox::critical(nullptr, QString("Character template invalid"), QString("Unable to read the character template: ") + appFile + QString(", the XML is invalid"));
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != TEMPLATEVALUES[TemplateType_template]))
    {
        qDebug() << "[CombatantFactory] Character template missing root item: " << appFile;
        QMessageBox::critical(nullptr, QString("Character template file invalid"), QString("Unable to read the character template: ") + appFile + QString(", the XML does not have the expected root item."));
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
        qDebug() << "[CombatantFactory] ERROR: Combatant template has no 'name' attribute";
}
