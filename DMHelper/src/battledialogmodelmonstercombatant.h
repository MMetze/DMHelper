#ifndef BATTLEDIALOGMODELMONSTERCOMBATANT_H
#define BATTLEDIALOGMODELMONSTERCOMBATANT_H

#include "battledialogmodelmonsterbase.h"
#include "scaledpixmap.h"
#include "combatant.h"
#include <QString>
#include <QPoint>

class Monster;
class MonsterClassv2;

class BattleDialogModelMonsterCombatant : public BattleDialogModelMonsterBase
{
    Q_OBJECT

public:
    BattleDialogModelMonsterCombatant(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelMonsterCombatant(Monster* monster);
    BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP);
    BattleDialogModelMonsterCombatant(Monster* monster, const QString& monsterName, int monsterSize, int monsterHP, int initiative, const QPointF& position);
    virtual ~BattleDialogModelMonsterCombatant() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    // Local
    virtual BattleDialogModelCombatant* clone() const override;
    virtual qreal getSizeFactor() const override;
    virtual int getSizeCategory() const override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual int getHitPoints() const override;
    virtual void setHitPoints(int hitPoints) override;
    virtual QString getName() const override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

    virtual int getMonsterType() const override;
    virtual MonsterClassv2* getMonsterClass() const override;

    void setMonsterName(const QString& monsterName);

    Monster* getMonster() const;
    void setMonster(Monster* monster);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _monsterSize;
    QString _monsterName;
    int _monsterHP;
};

#endif // BATTLEDIALOGMODELMONSTERCOMBATANT_H
