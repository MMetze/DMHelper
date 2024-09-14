#ifndef MAPFACTORY_H
#define MAPFACTORY_H

#include "objectfactory.h"

class MapFactory : public ObjectFactory
{
    Q_OBJECT
public:
    explicit MapFactory(QObject *parent = nullptr);

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

    virtual void configureFactory(const Ruleset& ruleset) override;
};

#endif // MAPFACTORY_H
