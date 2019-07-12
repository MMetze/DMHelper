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

    BattleDialogModelMonsterBase();
    explicit BattleDialogModelMonsterBase(Combatant* combatant);
    explicit BattleDialogModelMonsterBase(Combatant* combatant, int initiative, const QPointF& position);
    BattleDialogModelMonsterBase(const BattleDialogModelMonsterBase& other);
    virtual ~BattleDialogModelMonsterBase() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getType() const override;
    virtual int getMonsterType() const = 0;
    virtual MonsterClass* getMonsterClass() const = 0;

    virtual bool getShown() const override;
    virtual bool getKnown() const override;
    virtual int getSkillModifier(Combatant::Skills skill) const override;

    virtual int getLegendaryCount() const;

public slots:
    virtual void setShown(bool isShown);
    virtual void setKnown(bool isKnown);
    virtual void setLegendaryCount(int legendaryCount);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    //int convertSizeToFactor(const QString& monsterSize) const;

    bool _isShown;
    bool _isKnown;
    int _legendaryCount;
};

#endif // BATTLEDIALOGMODELMONSTERBASE_H
