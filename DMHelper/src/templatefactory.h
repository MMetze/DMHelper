#ifndef TEMPLATEFACTORY_H
#define TEMPLATEFACTORY_H

#include "objectfactory.h"
#include <QHash>

class DMHAttribute;
class TemplateObject;
class TemplateFrame;

class TemplateFactory : public ObjectFactory
{
    Q_OBJECT
public:

    // Template type and static variable declarations
    enum TemplateType
    {
        TemplateType_template = 0,
        TemplateType_attribute,
        TemplateType_element,
        TemplateType_string,
        TemplateType_integer,
        TemplateType_boolean,
        TemplateType_resource,
        TemplateType_dice,
        TemplateType_html,
        TemplateType_list,

        TEMPLATETYPE_COUNT
    };

    static const char* TEMPLATE_PROPERTY;
    static const char* TEMPLATE_WIDGET;

    static const char* TEMPLATEVALUES[TEMPLATETYPE_COUNT];

    // Constructors
    explicit TemplateFactory(QObject *parent = nullptr);

    // Static public members
    static QVariant convertStringToVariant(const QString& value, TemplateType type);
    static QString convertVariantToString(const QVariant& value, TemplateType type);

    // UI generation functionality
    static QWidget* loadUITemplate(const QString& templateFile);
    void readObjectData(QWidget* widget, TemplateObject* source, TemplateFrame* frame);
    void populateWidget(QWidget* widget, TemplateObject* source, TemplateFrame* frame, QHash<QString, QVariant>* hash = nullptr, QHash<QString, DMHAttribute>* hashAttributes = nullptr, int listIndex = 0, const QString& listKey = QString());
    QWidget* createResourceWidget(const QString& keyString, const QString& widgetString, const QString& templateFile);

    // Public members
    bool isEmpty() const;

    bool hasAttribute(const QString& name) const;
    DMHAttribute getAttribute(const QString& name) const;
    QHash<QString, DMHAttribute> getAttributes() const;

    bool hasElement(const QString& name) const;
    DMHAttribute getElement(const QString& name) const;
    QHash<QString, DMHAttribute> getElements() const;

    bool hasElementList(const QString& name) const;
    QHash<QString, DMHAttribute> getElementList(const QString& name) const;
    QHash<QString, QHash<QString, DMHAttribute>> getElementLists() const;

    bool hasEntry(const QString& name) const;
    QString getDefaultValue(const QString& keyString);

    TemplateObject* setDefaultValues(TemplateObject* object);
    static QString getAbsoluteTemplateFile(const QString& templateFile);

protected:
    void loadTemplate(const QString& templateFile);
    QWidget* createResourceWidgetFile(const QString& widgetFilename);
    QWidget* createResourceWidgetInternal(const QString& keyString);

    QHash<QString, DMHAttribute> _attributes;
    QHash<QString, DMHAttribute> _elements;
    QHash<QString, QHash<QString, DMHAttribute>> _elementLists;
};

class DMHAttribute
{
public:
    DMHAttribute() : _type(TemplateFactory::TemplateType_string), _default() {}
    DMHAttribute(TemplateFactory::TemplateType type, const QString& defaultValue) : _type(type), _default(defaultValue) {}

    TemplateFactory::TemplateType _type;
    QString _default;
};

#endif // TEMPLATEFACTORY_H
