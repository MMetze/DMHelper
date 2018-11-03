#ifndef ADVENTURE_H
#define ADVENTURE_H

#include "campaignobjectbase.h"
#include <QList>

class Map;
class Encounter;
class QDomDocument;
class QDomElement;

class Adventure : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Adventure(const QString& adventureName, QObject *parent = 0);
    explicit Adventure(const QDomElement& element, QObject *parent = 0);
    ~Adventure();

    int getEncounterCount();
    Encounter* getEncounterById(int id);
    Encounter* getEncounterByIndex(int index);
    int addEncounter(Encounter* newItem);
    int addEncounter(Encounter* newItem, int index);
    Encounter* removeEncounter(int id);
    void moveEncounterTo(int id, int index);

    int getMapCount();
    Map* getMapById(int id);
    Map* getMapByIndex(int index);
    int addMap(Map* newItem);
    int addMap(Map* newItem, int index);
    Map* removeMap(int id);
    void moveMapTo(int id, int index);

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory);
    virtual void inputXML(const QDomElement &element);
    virtual void postProcessXML(const QDomElement &element);

    QString getName() const;
    void setName(const QString& adventureName);

    bool getExpanded() const;
    void setExpanded(bool expanded);

signals:
    void changed();
    void dirty();

public slots:

private:
    QString _name;
    QList<Encounter*> encounters;
    QList<Map*> maps;

    bool _expanded;
};

#endif // ADVENTURE_H
