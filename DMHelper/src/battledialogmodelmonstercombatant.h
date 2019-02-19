#ifndef BATTLEDIALOGMODELMONSTERCOMBATANT_H
#define BATTLEDIALOGMODELMONSTERCOMBATANT_H

#include "battledialogmodelmonsterbase.h"
#include "scaledpixmap.h"
#include "combatant.h"
#include <QString>
#include <QPoint>

class Monster;
class MonsterClass;

class BattleDialogModelMonsterCombatant : public BattleDialogModelMonsterBase
{
    Q_OBJECT

public:
    BattleDialogModelMonsterCombatant();
    BattleDialogModelMonsterCombatant(Monster* monster);
    BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP);
    BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP, int initiative, const QPointF& position);
    BattleDialogModelMonsterCombatant(const BattleDialogModelMonsterCombatant& other);
    virtual ~BattleDialogModelMonsterCombatant();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // Local
    virtual BattleDialogModelCombatant* clone() const;
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

    Monster* getMonster() const;
    void setMonster(Monster* monster);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

    int _monsterSize;
    QString _monsterName;
    int _monsterHP;
};

#endif // BATTLEDIALOGMODELMONSTERCOMBATANT_H
