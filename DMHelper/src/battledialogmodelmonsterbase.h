#ifndef BATTLEDIALOGMODELMONSTERBASE_H
#define BATTLEDIALOGMODELMONSTERBASE_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include <QString>
#include <QStringList>
#include <QHash>

class MonsterClassv2;

class BattleDialogModelMonsterBase : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    enum
    {
        BattleMonsterType_Base = 0,
        BattleMonsterType_Combatant,
        BattleMonsterType_Class,
    };

    BattleDialogModelMonsterBase(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelMonsterBase(Combatant* combatant);
    explicit BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position);
    virtual ~BattleDialogModelMonsterBase() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual int getCombatantType() const override;
    virtual int getMonsterType() const = 0;
    virtual MonsterClassv2* getMonsterClass() const = 0;

    virtual void setMonsterName(const QString& monsterName) = 0;

    virtual int getSkillModifier(Combatant::Skills skill) const override;
    virtual QStringList getConditionList() const override;
    virtual bool hasConditionId(const QString& conditionId) const override;

    virtual int getLegendaryCount() const;

    // Per-instance per-round resource counters (legendary actions + recharge abilities, etc.).
    // The current value is stored here; the resource definition (max, recharge schedule)
    // lives on the MonsterClassv2 via getPerRoundResources().
    int  getResourceCount(const QString& resourceName) const;
    bool hasResourceCounter(const QString& resourceName) const;
    QStringList getResourceCounterNames() const;

signals:
    void dataChanged(BattleDialogModelMonsterBase* monsterBase);
    void imageChanged(BattleDialogModelMonsterBase* monsterBase);
    void resourceCountChanged(BattleDialogModelMonsterBase* monsterBase, const QString& resourceName, int newValue);

public slots:
    virtual void setConditionList(const QStringList& conditions) override;
    virtual void addConditionId(const QString& conditionId) override;
    virtual void removeConditionId(const QString& conditionId) override;
    virtual void clearConditions() override;
    virtual void setLegendaryCount(int legendaryCount);
    void setResourceCount(const QString& resourceName, int count);
    void resetResources(const QString& scope);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _legendaryCount;
    QStringList _conditionList;
    QHash<QString, int> _resourceCounters;
};

#endif // BATTLEDIALOGMODELMONSTERBASE_H
