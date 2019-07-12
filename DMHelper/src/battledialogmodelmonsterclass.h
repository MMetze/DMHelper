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
    explicit BattleDialogModelMonsterClass(MonsterClass* monsterClass);
    explicit BattleDialogModelMonsterClass(MonsterClass* monsterClass, const QString& monsterName, int initiative, const QPointF& position);
    BattleDialogModelMonsterClass(const BattleDialogModelMonsterClass& other);
    virtual ~BattleDialogModelMonsterClass() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    // Local
    virtual BattleDialogModelMonsterClass* clone() const override;
    virtual int getSizeFactor() const override;
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
    virtual MonsterClass* getMonsterClass() const override;

    void setMonsterName(const QString& monsterName);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    MonsterClass* _monsterClass;
    QString _monsterName;
    int _monsterHP;
};


#endif // BATTLEDIALOGMODELMONSTERCLASS_H
