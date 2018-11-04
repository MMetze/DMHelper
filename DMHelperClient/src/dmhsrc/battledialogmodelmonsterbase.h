#ifndef BATTLEDIALOGMODELMONSTERBASE_H
#define BATTLEDIALOGMODELMONSTERBASE_H

#include "battledialogmodelcombatant.h"
//#include "combatant.h"
#include <QString>

//class MonsterClass;

class BattleDialogModelMonsterBase : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    /*
    enum
    {
        BattleMonsterType_Base = 0,
        BattleMonsterType_Combatant,
        BattleMonsterType_Class,
    };
    */

    BattleDialogModelMonsterBase();
    BattleDialogModelMonsterBase(int initiative, const QPointF& position);
    BattleDialogModelMonsterBase(const BattleDialogModelMonsterBase& other);
    virtual ~BattleDialogModelMonsterBase();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    virtual int getType() const;
//    virtual int getMonsterType() const = 0;
//    virtual MonsterClass* getMonsterClass() const = 0;
    virtual BattleDialogModelCombatant* clone() const;

    virtual bool getShown() const;
    virtual bool getKnown() const;
//    virtual int getSkillModifier(Combatant::Skills skill) const;

//    virtual int getLegendaryCount() const;
    virtual int getSizeFactor() const;
    virtual int getSizeCategory() const;

    virtual QString getName() const;

public slots:
    virtual void setShown(bool isShown);
    virtual void setKnown(bool isKnown);
//    virtual void setLegendaryCount(int legendaryCount);

protected:
    // From BattleDialogModelCombatant
//    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

    //int convertSizeToFactor(const QString& monsterSize) const;

    bool _isShown;
    bool _isKnown;
    //int _legendaryCount;
};

#endif // BATTLEDIALOGMODELMONSTERBASE_H
