#ifndef EXPORTWORKER_H
#define EXPORTWORKER_H

#include <QObject>
#include <QDir>

class Campaign;
class EncounterBattle;
class BattleDialogModelEffect;
class CampaignObjectBase;
class QTreeWidgetItem;
class Spell;
class MonsterClassv2;
class QDomDocument;
class QDomElement;

class ExportWorker : public QObject
{
    Q_OBJECT
public:
    explicit ExportWorker(Campaign& campaign, QTreeWidgetItem* widgetItem, QDir& directory, const QString& exportName, QObject *parent = nullptr);

public slots:
    void doWork();

    void setExportFlags(bool campaign, bool references, bool soundboard, bool monsters, bool spells);
    void setSpellExports(QStringList spellList);
    void setMonsterExports(QStringList monsterList);

signals:
    void workComplete(bool success);

protected slots:
    void recursiveExport(QTreeWidgetItem* widgetItem, QDomDocument &doc, QDomElement &parentElement, QDir& directory);
    void exportObjectAssets(const CampaignObjectBase* object, QDir& directory, QDomDocument &doc, QDomElement &element);
    void exportBattle(const EncounterBattle* battle, QDir& directory, QDomElement &battleElement);
    void exportMonster(QDomDocument &doc, QDomElement& bestiaryElement, const QString& monsterName, QDir& directory);
    void exportMonster(QDomDocument &doc, QDomElement& bestiaryElement, MonsterClassv2* monster, QDir& directory);
    void exportSpell(QDomDocument &doc, QDomElement& spellbookElement, const QString& spellName, QDir& directory);
    QString exportFile(const QString& filename, const QDir& directory, QDomElement& element, const QString& fileAttribute, bool hashfileNaming);

protected:
    bool tryToDoWork();
    QString exportHashedFile(const QString& filename, const QDir& directory);
    QString exportDirectFile(const QString& filename, const QDir& directory);
    QDomElement findEffectElement(QDomElement& parentElement, BattleDialogModelEffect* effect);

    Campaign& _campaign;
    QTreeWidgetItem* _widgetItem;
    QString _exportName;
    bool _campaignExport;
    bool _referencesExport;
    bool _soundboardExport;
    bool _monstersExport;
    bool _spellsExport;
    QStringList _spellList;
    QStringList _monsterList;
    QList<Spell*> _exportedSpells;
    QList<MonsterClassv2*> _exportedMonsters;
    QHash<QString, QString> _exportedFiles;

    QDir _directory;
};

#endif // EXPORTWORKER_H
