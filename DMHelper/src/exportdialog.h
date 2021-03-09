#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class Campaign;
class CampaignObjectBase;
class EncounterBattle;
class BattleDialogModelEffect;
class QUuid;
class QTreeWidgetItem;
class QDir;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(const Campaign& campaign, const QUuid& selectedItem, QWidget *parent = nullptr);
    virtual ~ExportDialog() override;

    QTreeWidgetItem* createChildObject(const CampaignObjectBase* childObject, const QUuid& selectedItem);

private slots:
    void handleCampaignItemChanged(QTreeWidgetItem *item, int column);
    void runExport();

private:
    void setRecursiveChecked(QTreeWidgetItem *item, bool checked);
    void setObjectIcon(const CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem);
    void checkObjectContent(const CampaignObjectBase* object);
    void refreshMonsters();
    void recursiveRefreshMonsters(QTreeWidgetItem* widgetItem);
    void recursiveExport(QTreeWidgetItem* widgetItem, const QDir& directory);
    void exportObjectAssets(const CampaignObjectBase* object, const QDir& directory);
    void exportBattle(const EncounterBattle* battle, const QDir& directory);
    void exportFile(const QString& filename, const QDir& directory);

    Ui::ExportDialog *ui;

    const Campaign& _campaign;
    const QUuid& _selectedItem;
    QStringList _monsters;

};

#endif // EXPORTDIALOG_H
