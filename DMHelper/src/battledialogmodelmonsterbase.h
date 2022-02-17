#ifndef BATTLEDIALOGMODELMONSTERBASE_H
#define BATTLEDIALOGMODELMONSTERBASE_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include <QString>

class MonsterClass;

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

    virtual int getCombatantType() const override;
    virtual int getMonsterType() const = 0;
    virtual MonsterClass* getMonsterClass() const = 0;

    virtual int getSkillModifier(Combatant::Skills skill) const override;
    virtual int getConditions() const override;
    virtual bool hasCondition(Combatant::Condition condition) const override;

    virtual int getLegendaryCount() const;

signals:
    void dataChanged(BattleDialogModelMonsterBase* monsterBase);

public slots:
    virtual void setConditions(int conditions) override;
    virtual void applyConditions(int conditions) override;
    virtual void removeConditions(int conditions) override;
    virtual void clearConditions() override;
    virtual void setLegendaryCount(int legendaryCount);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _legendaryCount;
    int _conditions;
};

#endif // BATTLEDIALOGMODELMONSTERBASE_H
