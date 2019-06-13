#ifndef OBJECTIMPORTER_H
#define OBJECTIMPORTER_H

#include <QObject>
#include <QUuid>

class Campaign;
class QStandardItem;
class QDomElement;

class ObjectImporter : public QObject
{
    Q_OBJECT
public:
    explicit ObjectImporter(QObject *parent = nullptr);

signals:

public slots:
    QUuid importObject(Campaign& campaign, QStandardItem* item);

protected:
    QUuid importCombatant(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importEncounter(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importMap(Campaign& campaign, QStandardItem* item, QDomElement& element);
    QUuid importAdventure(Campaign& campaign, QStandardItem* item, QDomElement& element);

    bool isWorldEntry(QStandardItem* item);
    QStandardItem* findParentbyType(QStandardItem* child, int parentType);
};

#endif // OBJECTIMPORTER_H
