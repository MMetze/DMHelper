#ifndef ENCOUNTERFACTORY_H
#define ENCOUNTERFACTORY_H

#include "objectfactory.h"

class CampaignObjectBase;
class QDomElement;
class QString;

class EncounterFactory : public ObjectFactory
{
    Q_OBJECT
public:
    explicit EncounterFactory(QObject *parent = nullptr);

    //static CampaignObjectBase* createEncounter(int objectType, const QString& objectName);
    //static CampaignObjectBase* createEncounter(const QDomElement& element, bool isImport);

    //static Encounter* cloneEncounter(Encounter& encounter);

signals:

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

private:
};

#endif // ENCOUNTERFACTORY_H
