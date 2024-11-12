#ifndef MONSTERFACTORY_H
#define MONSTERFACTORY_H

#include "templatefactory.h"

class MonsterFactory : public TemplateFactory
{
    Q_OBJECT
public:
    explicit MonsterFactory(QObject *parent = nullptr);

    static MonsterFactory* Instance();
    static void Shutdown();

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

    virtual void configureFactory(const Ruleset& ruleset, int inputMajorVersion, int inputMinorVersion) override;

protected:

    static MonsterFactory* _instance;

    bool _compatibilityMode;
};

#endif // MONSTERFACTORY_H
