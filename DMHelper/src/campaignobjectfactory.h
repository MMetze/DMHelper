#ifndef CAMPAIGNOBJECTFACTORY_H
#define CAMPAIGNOBJECTFACTORY_H

#include <QObject>
#include <QList>

class QDomElement;
class CampaignObjectBase;
class ObjectFactory;

class CampaignObjectFactory : public QObject
{
    Q_OBJECT
public:
    explicit CampaignObjectFactory(QObject *parent = nullptr);
    ~CampaignObjectFactory();

    static void addFactory(ObjectFactory* factory);
    static CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport);
    static CampaignObjectBase* createObject(const QDomElement& element, bool isImport);

private:
    int factoryCount() const;
    void addFactoryPrivate(ObjectFactory* factory);
    ObjectFactory* getFactory(int index) const;

    static CampaignObjectFactory* Instance();
    static CampaignObjectFactory* _instance;

    CampaignObjectBase* localCreateObject(const QDomElement& element, bool isImport);

    QList<ObjectFactory*> _factoryList;
};

#endif // CAMPAIGNOBJECTFACTORY_H
