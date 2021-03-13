#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QDir>

class EncounterBattle;
class BattleDialogModelEffect;
class CampaignObjectBase;
class QTreeWidgetItem;

class ExportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ExportWorker(QTreeWidgetItem* widgetItem, const QDir& directory, QObject *parent = nullptr);

public slots:
    void doWork();

signals:
    void workComplete(bool success);

protected slots:
    bool recursiveExport(QTreeWidgetItem* widgetItem, const QDir& directory);
    bool exportObjectAssets(const CampaignObjectBase* object, const QDir& directory);
    void exportBattle(const EncounterBattle* battle, const QDir& directory);
    void exportFile(const QString& filename, const QDir& directory);

protected:
    QTreeWidgetItem* _widgetItem;
    QDir _directory;

};

#endif // EXPORTWORKER_H
