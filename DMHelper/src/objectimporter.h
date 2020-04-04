#ifndef OBJECTIMPORTER_H
#define OBJECTIMPORTER_H

#include <QObject>
#include <QUuid>
#include <QStringList>

class Campaign;
class QStandardItem;
class QDomElement;
class CampaignObjectBase;

class ObjectImporter : public QObject
{
    Q_OBJECT
public:
    explicit ObjectImporter(QObject *parent = nullptr);

signals:

public slots:
    bool importObject(Campaign& campaign);

protected:
    bool checkObjectDuplicates(CampaignObjectBase* object, Campaign& targetCampaign, Campaign& importCampaign);
    /*
    QUuid importCombatant(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importEncounter(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importMap(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importAdventure(Campaign& campaign, QStandardItem* item, QDomElement& element);

    bool isWorldEntry(QStandardItem* item);
    QStandardItem* findParentbyType(QStandardItem* child, int parentType);
    */

    QStringList _duplicateObjects;
};

#endif // OBJECTIMPORTER_H
