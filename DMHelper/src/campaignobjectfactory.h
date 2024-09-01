#ifndef CAMPAIGNOBJECTFACTORY_H
#define CAMPAIGNOBJECTFACTORY_H

#include <QObject>
#include <QList>

class QDomElement;
class CampaignObjectBase;
class ObjectFactory;
class Ruleset;

class CampaignObjectFactory : public QObject
{
    Q_OBJECT
public:
    explicit CampaignObjectFactory(QObject *parent = nullptr);
    ~CampaignObjectFactory();

    static CampaignObjectFactory* Instance();
    static void Shutdown();

    static void configureFactories(const Ruleset& ruleset);

    static CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport);
    static CampaignObjectBase* createObject(const QDomElement& element, bool isImport);

    int factoryCount() const;
    ObjectFactory* getFactory(int index) const;

private:
    static CampaignObjectFactory* _instance;

    CampaignObjectBase* localCreateObject(const QDomElement& element, bool isImport);

    QList<ObjectFactory*> _factoryList;
};

#endif // CAMPAIGNOBJECTFACTORY_H
