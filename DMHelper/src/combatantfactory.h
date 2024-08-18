#ifndef COMBATANTFACTORY_H
#define COMBATANTFACTORY_H

#include "objectfactory.h"
#include "combatant.h"
#include <QVariantMap>

class QDomElement;
class QString;
class DMHAttribute;

class CombatantFactory : public ObjectFactory
{
    Q_OBJECT
public:

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

    explicit CombatantFactory(QObject *parent = nullptr);

    static CombatantFactory* Instance();
    static void Shutdown();

    static Combatant* createCombatant(int combatantType, const QDomElement& element, bool isImport, QObject *parent = nullptr);

    static QVariant convertStringToVariant(const QString& value, TemplateType type);
    static QString convertVariantToString(const QVariant& value, TemplateType type);

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

signals:

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

private:
    void loadCharacterTemplate();

    static CombatantFactory* _instance;

    QHash<QString, DMHAttribute> _attributes;
    QHash<QString, DMHAttribute> _elements;
    QHash<QString, QHash<QString, DMHAttribute>> _elementLists;
};

class DMHAttribute
{
public:
    DMHAttribute() : _type(CombatantFactory::TemplateType_string), _default() {}
    DMHAttribute(CombatantFactory::TemplateType type, const QString& defaultValue) : _type(type), _default(defaultValue) {}

    CombatantFactory::TemplateType _type;
    QString _default;
};

#endif // COMBATANTFACTORY_H
