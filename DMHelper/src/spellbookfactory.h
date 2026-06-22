#ifndef SPELLBOOKFACTORY_H
#define SPELLBOOKFACTORY_H

#include "templatefactory.h"

class Ruleset;

// Factory for Spellv2 attribute templates. Mirrors MonsterFactory but loads
// spell field definitions from the per-ruleset spell data XML (e.g. spell5e.xml).
class SpellbookFactory : public TemplateFactory
{
    Q_OBJECT
public:
    explicit SpellbookFactory(QObject *parent = nullptr);

    static SpellbookFactory* Instance();
    static void Shutdown();

    QString getRulesetName() const;

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

    virtual void configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion) override;

protected:
    static SpellbookFactory* _instance;

    bool _compatibilityMode;
    QString _rulesetName;
};

#endif // SPELLBOOKFACTORY_H
