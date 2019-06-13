#ifndef ADVENTURE_H
#define ADVENTURE_H

#include "campaignobjectbase.h"
#include <QList>
#include <QUuid>

class Map;
class Encounter;
class QDomDocument;
class QDomElement;

class Adventure : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Adventure(const QString& adventureName, QObject *parent = nullptr);
    explicit Adventure(const QDomElement& element, bool isImport, QObject *parent = nullptr);
    ~Adventure();

    int getEncounterCount();
    Encounter* getEncounterById(QUuid id);
    Encounter* getEncounterByIndex(int index);
    QUuid addEncounter(Encounter* newItem);
    QUuid addEncounter(Encounter* newItem, int index);
    Encounter* removeEncounter(QUuid id);
    void moveEncounterTo(QUuid id, int index);

    int getMapCount();
    Map* getMapById(QUuid id);
    Map* getMapByIndex(int index);
    QUuid addMap(Map* newItem);
    QUuid addMap(Map* newItem, int index);
    Map* removeMap(QUuid id);
    void moveMapTo(QUuid id, int index);

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual void postProcessXML(const QDomElement &element, bool isImport);

    QString getName() const;
    void setName(const QString& adventureName);

    bool getExpanded() const;
    void setExpanded(bool expanded);

    void clear();

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
