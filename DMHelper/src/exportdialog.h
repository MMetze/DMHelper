#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class Campaign;
class CampaignObjectBase;
class QUuid;
class QTreeWidgetItem;
class ExportWorker;
class DMHWaitingDialog;
class MonsterClassv2;
class Spell;
class Characterv2;
class QThread;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(Campaign& campaign, const QUuid& selectedItem, QWidget *parent = nullptr);
    virtual ~ExportDialog() override;

    QTreeWidgetItem* createChildObject(CampaignObjectBase* childObject);

signals:
    void startWork();

private slots:
    void handleCampaignItemChanged(QTreeWidgetItem *item, int column);
    void addMonsters();
    void addSpells();
    void runExport();
    void exportFinished(bool success);
    void threadFinished();

private:
    void setRecursiveChecked(QTreeWidgetItem *item, bool checked);
    void setRecursiveParentChecked(QTreeWidgetItem *item);
    void setObjectIcon(CampaignObjectBase* baseObject, QTreeWidgetItem* widgetItem);
    void checkCharacters();
    void refreshMonsters();
    void recursiveRefreshMonsters(QTreeWidgetItem* widgetItem);
    void checkObjectContent(CampaignObjectBase* object);

    void checkItem(CampaignObjectBase* object);
    void checkItem(QUuid id);
    QTreeWidgetItem* findItem(QTreeWidgetItem *item, QUuid id);

    void addCharacter(Characterv2* character);
    void addMonster(MonsterClassv2* monsterClass);
    void addSpell(Spell* spell);

    Ui::ExportDialog *ui;

    Campaign& _campaign;
    const QUuid& _selectedItem;
    QStringList _monsters;
    QStringList _spells;
    QList<Characterv2*> _characters;

    QThread* _workerThread;
    ExportWorker* _worker;
    DMHWaitingDialog* _waitingDlg;

};

#endif // EXPORTDIALOG_H
