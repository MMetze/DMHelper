#ifndef BATTLEDIALOGMODELMONSTERCLASS_H
#define BATTLEDIALOGMODELMONSTERCLASS_H

#include "battledialogmodelmonsterbase.h"
#include "scaledpixmap.h"
#include <QString>
#include <QPoint>

class MonsterClass;

class BattleDialogModelMonsterClass : public BattleDialogModelMonsterBase
{
    Q_OBJECT

public:
    BattleDialogModelMonsterClass();
    BattleDialogModelMonsterClass(MonsterClass* monsterClass);
    BattleDialogModelMonsterClass(MonsterClass* monsterClass, const QString& monsterName, int initiative, const QPointF& position);
    BattleDialogModelMonsterClass(const BattleDialogModelMonsterClass& other);
    virtual ~BattleDialogModelMonsterClass();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // Local
    virtual BattleDialogModelMonsterClass* clone() const;
    virtual int getSizeFactor() const;
    virtual int getSizeCategory() const;

    virtual int getStrength() const;
    virtual int getDexterity() const;
    virtual int getConstitution() const;
    virtual int getIntelligence() const;
    virtual int getWisdom() const;
    virtual int getCharisma() const;

    virtual int getSpeed() const;
    virtual int getArmorClass() const;
    virtual int getHitPoints() const;
    virtual void setHitPoints(int hitPoints);
    virtual QString getName() const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const;

    virtual int getMonsterType() const;
    virtual MonsterClass* getMonsterClass() const;

    void setMonsterName(const QString& monsterName);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

    MonsterClass* _monsterClass;
    QString _monsterName;
    int _monsterHP;
};


#endif // BATTLEDIALOGMODELMONSTERCLASS_H
