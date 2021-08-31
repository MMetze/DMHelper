#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QDir>

class EncounterBattle;
class BattleDialogModelEffect;
class CampaignObjectBase;
class QTreeWidgetItem;
class Spell;
class MonsterClass;

class ExportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ExportWorker(QTreeWidgetItem* widgetItem, const QDir& directory, bool fullExport, QObject *parent = nullptr);

public slots:
    void doWork();

    void setSpellExports(QStringList spellList);
    void setMonsterExports(QStringList monsterList);

signals:
    void workComplete(bool success);

protected slots:
    bool recursiveExport(QTreeWidgetItem* widgetItem, const QDir& directory);
    bool exportObjectAssets(const CampaignObjectBase* object, const QDir& directory);
    void exportBattle(const EncounterBattle* battle, const QDir& directory);
    void exportMonster(const QString& monsterName, const QDir& directory);
    void exportMonster(MonsterClass* monster, const QDir& directory);
    void exportSpell(const QString& spellName, const QDir& directory);
    void exportFile(const QString& filename, const QDir& directory, bool hashfileNaming);

protected:
    QTreeWidgetItem* _widgetItem;
    QStringList _spellList;
    QStringList _monsterList;
    QList<Spell*> _exportedSpells;
    QList<MonsterClass*> _exportedMonsters;
    QDir _directory;
    bool _fullExport;
};

#endif // EXPORTWORKER_H
