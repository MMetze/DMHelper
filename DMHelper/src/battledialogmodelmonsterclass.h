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
    BattleDialogModelMonsterClass(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelMonsterClass(MonsterClass* monsterClass);
    explicit BattleDialogModelMonsterClass(MonsterClass* monsterClass, const QString& monsterName, int initiative, const QPointF& position);
    virtual ~BattleDialogModelMonsterClass() override;

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
    virtual MonsterClass* getMonsterClass() const override;

    void setSizeFactor(qreal sizeFactor);
    void setMonsterName(const QString& monsterName);
    void setIconIndex(int index);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    MonsterClass* _monsterClass;
    QString _monsterName;
    int _monsterHP;
    qreal _monsterSize;
    int _iconIndex;
};


#endif // BATTLEDIALOGMODELMONSTERCLASS_H
