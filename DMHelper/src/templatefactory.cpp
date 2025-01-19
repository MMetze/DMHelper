#include "templatefactory.h"
#include "templateobject.h"
#include "templateframe.h"
#include "templateresourcelayout.h"
#include "dice.h"
#include "combatant.h"
#include "rulefactory.h"
#include <QVariant>
#include <QCoreApplication>
#include <QDomElement>
#include <QCoreApplication>
#include <QUiLoader>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollArea>
#include <QLayout>
#include <QLabel>
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

QWidget* TemplateFactory::loadUITemplate(const QString& templateFile)
{
    QWidget* result = nullptr;

    if(templateFile.isEmpty())
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: No template file name provided, cannot load any UI template files";
        return result;
    }

    if(!RuleFactory::Instance())
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: No rule factory exists, cannot load any UI template files: " << templateFile;
        return result;
    }

    // Try our best to load the given character template file
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
                qDebug() << "[RuleFactory::loadUITemplate] ERROR: Relative UI Template File not found: " << templateFile;
                return result;
            }
        }
    }
    else
    {
        appFile = templateFile;
        if(!QFileInfo::exists(appFile))
        {
            qDebug() << "[RuleFactory::loadUITemplate] ERROR: Absolute Character UI Template File not found: " << templateFile;
            return result;
        }
    }

    QUiLoader loader;
    QFile file(appFile);
    file.open(QFile::ReadOnly);
    result = loader.load(&file);
    file.close();

    if(!result)
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: UI Template File " << appFile << " could not be loaded: " << loader.errorString();
        return result;
    }

    qDebug() << "[RuleFactory::loadUITemplate] UI Template File " << appFile << " loaded.";

    // Activate hyperlinks for any included text edits
    QList<QTextEdit*> textEdits = result->findChildren<QTextEdit*>();
    for(QTextEdit* edit : textEdits)
    {
        auto &clist = edit->children();
        for(QObject *pObj : clist)
        {
            QString cname = pObj->metaObject()->className();
            if(cname == "QWidgetTextControl")
                pObj->setProperty("openExternalLinks", true);
        }
    }

    return result;
}

void TemplateFactory::readObjectData(QWidget* widget, TemplateObject* source, TemplateFrame* frame)
{
    if((!widget) || (!frame))
        return;

    // Walk through the loaded UI Widget and allocate the appropriate values to the UI elements
    populateWidget(widget, source, frame, nullptr);

    QList<QScrollArea*> scrollAreas = widget->findChildren<QScrollArea*>();
    for(auto scrollArea : scrollAreas)
    {
        if(!scrollArea)
            continue;

        QString keyString = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        QString widgetString = scrollArea->property(TemplateFactory::TEMPLATE_WIDGET).toString();
        if((!keyString.isEmpty()) && (!widgetString.isEmpty()))
        {
            if(QWidget* oldWidget = scrollArea->takeWidget())
            {
                if(QLayout* oldLayout = oldWidget->layout())
                {
                    QLayoutItem *child;
                    while((child = scrollArea->layout()->takeAt(0)) != nullptr)
                    {
                        if(child->widget())
                            child->widget()->deleteLater();
                        delete child;
                    }
                    delete oldLayout;
                }
                oldWidget->deleteLater();
            }

            scrollArea->setWidgetResizable(true);
            QFrame* scrollWidget = new QFrame;
            QVBoxLayout* scrollLayout = new QVBoxLayout;
            scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            scrollWidget->setLayout(scrollLayout);
            scrollArea->setWidget(scrollWidget);

            QList<QVariant> listValue = source->getListValue(keyString);
            if(!listValue.isEmpty())
            {
                for(int i = 0; i < listValue.count(); ++i)
                {
                    QVariant listEntry = listValue.at(i);
                    if(listEntry.isNull())
                        continue;

                    QWidget* newWidget = createResourceWidget(keyString, widgetString);
                    if(!newWidget)
                    {
                        qDebug() << "[TemplateFactory] ERROR: Unable to create the object widget: " << widgetString << ", for scroll area: " << keyString;
                        return;
                    }

                    QHash<QString, QVariant> hashValue = listEntry.toHash();

                    // Walk through the loaded UI Widget and allocate the appropriate object values to the UI elements
                    populateWidget(newWidget, nullptr, frame, &hashValue, i, keyString);

                    newWidget->installEventFilter(this);
                    scrollLayout->addWidget(newWidget);
                }
            }
            scrollArea->installEventFilter(this);
        }
    }
}

void TemplateFactory::populateWidget(QWidget* widget, TemplateObject* source, TemplateFrame* templateFrame, QHash<QString, QVariant>* hash, int listIndex, const QString& listKey)
{
    if((!widget) || (!templateFrame))
        return;

    QList<QLineEdit*> lineEdits = widget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;

        QString keyString = lineEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString;
            if(source)
                valueString = source->getValueAsString(keyString);
            else if(hash)
                valueString = hash->value(keyString).toString();

            lineEdit->setText(valueString.isEmpty() ? getDefaultValue(keyString) : valueString);
            connect(lineEdit, &QLineEdit::editingFinished, [templateFrame, source, lineEdit](){templateFrame->handleEditBoxChange(lineEdit, source, lineEdit->text());});
        }
    }

    QList<QTextEdit*> textEdits = widget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if(!textEdit)
            continue;

        QString keyString = textEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString;
            if(source)
                valueString = source->getValueAsString(keyString);
            else if(hash)
                valueString = hash->value(keyString).toString();

            textEdit->setHtml(valueString.isEmpty() ? getDefaultValue(keyString) : valueString);
            //if(!valueString.isEmpty())
            connect(textEdit, &QTextEdit::textChanged, [templateFrame, source, textEdit](){templateFrame->handleEditBoxChange(textEdit, source, textEdit->toHtml());});
        }
    }

    QList<QFrame*> frames = widget->findChildren<QFrame*>();
    for(auto frame : frames)
    {
        if((!frame) || (dynamic_cast<QTextEdit*>(frame)) || (dynamic_cast<QScrollArea*>(frame)) || (dynamic_cast<QScrollArea*>(frame->parentWidget())))
            continue;

        if(QLayout* oldLayout = frame->layout())
        {
            frame->removeEventFilter(oldLayout);
            delete oldLayout;
        }

        QString keyString = frame->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        ResourcePair valuePair;
        if(source)
            valuePair = source->getResourceValue(keyString);
        else if(hash)
            valuePair = hash->value(keyString).value<ResourcePair>();

        TemplateResourceLayout* layout = nullptr;
        if(hash)
        {
            layout = new TemplateResourceLayout(listKey, listIndex, keyString, valuePair);
            connect(layout, &TemplateResourceLayout::resourceListValueChanged, [source](const QString& key, int listIndex, const QString& listKey, const QVariant& value){ source->setListValue(key, listIndex, listKey, value); });
            connect(layout, &TemplateResourceLayout::addResource, [templateFrame, source, widget](){ templateFrame->handleAddResource(widget, source); });
            connect(layout, &TemplateResourceLayout::removeResource, [templateFrame, source, widget](){ templateFrame->handleRemoveResource(widget, source); });
        }
        else // character or nulls
        {
            layout = new TemplateResourceLayout(keyString, valuePair);
            connect(layout, &TemplateResourceLayout::resourceValueChanged, [source](const QString& key, const ResourcePair& value){ source->setResourceValue(key, value); });
        }
        frame->installEventFilter(layout);
        frame->setLayout(layout);
    }
}

QWidget* TemplateFactory::createResourceWidget(const QString& keyString, const QString& widgetString)
{
    if(widgetString.isEmpty())
        return createResourceWidgetInternal(keyString);

#ifdef Q_OS_MAC
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    fileDirPath.cdUp();
    QString appFile = fileDirPath.path() + QString("/Resources/ui/") + widgetString;
#else
    QDir fileDirPath(QCoreApplication::applicationDirPath());
    QString appFile = fileDirPath.path() + QString("/resources/ui/") + widgetString;
#endif

    if(QFileInfo::exists(appFile))
    {
        return createResourceWidgetFile(appFile);
    }
    else
    {
        qDebug() << "[TemplateFactory] ERROR: UI Widget Template File not found: " << appFile << ", for the widget name: " << widgetString;
        return createResourceWidgetInternal(keyString);
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

QString TemplateFactory::getDefaultValue(const QString& keyString)
{
    if(hasAttribute(keyString))
        return getAttribute(keyString)._default;
    else if(hasElement(keyString))
        return getElement(keyString)._default;
    else
        return QString();
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

QWidget* TemplateFactory::createResourceWidgetFile(const QString& widgetFilename)
{
    if(widgetFilename.isEmpty())
        return nullptr;

    QUiLoader loader;
    QFile file(widgetFilename);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "[TemplateFactory] ERROR: Unable to open UI Widget Template File: " << widgetFilename;
        return nullptr;
    }

    QWidget* newWidget = loader.load(&file);
    file.close();

    if(!newWidget)
        qDebug() << "[TemplateFactory] ERROR: UI Widget Template File could not be loaded: " << widgetFilename << ", error: " << loader.errorString();

    return newWidget;
}

QWidget* TemplateFactory::createResourceWidgetInternal(const QString& keyString)
{
    QHash<QString, DMHAttribute> elementList = getElementList(keyString);
    if(elementList.isEmpty())
        return nullptr;

    // Interate through the keys of the elementList and create the appropriate UI elements
    QWidget* newWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(newWidget);

    QStringList keys = elementList.keys();
    for(const auto &elementName : keys)
    {
        if(elementName.isEmpty())
            continue;

        layout->addWidget(new QLabel(elementName));
        QWidget* lineEditWidget = new QLineEdit();
        lineEditWidget->setProperty(TemplateFactory::TEMPLATE_PROPERTY, keyString);
        layout->addWidget(lineEditWidget);
    }

    return newWidget;
}
