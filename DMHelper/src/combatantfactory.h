#ifndef COMBATANTFACTORY_H
#define COMBATANTFACTORY_H

#include "templatefactory.h"

class QDomElement;
class QString;

class CombatantFactory : public TemplateFactory
{
    Q_OBJECT
public:

    explicit CombatantFactory(QObject *parent = nullptr);

    static CombatantFactory* Instance();
    static void Shutdown();

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

    virtual void configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion) override;

    CampaignObjectBase* setDefaultValues(CampaignObjectBase* object);

protected:

    static CombatantFactory* _instance;

    bool _compatibilityMode;
};

#endif // COMBATANTFACTORY_H
