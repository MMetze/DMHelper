#ifndef BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H
#define BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H

#include "battledialogmodelcombatant.h"
#include <QUuid>

class BattleDialogModelCombatantDownload : public BattleDialogModelCombatant
{
    Q_OBJECT
public:
    BattleDialogModelCombatantDownload(QObject *parent = nullptr);

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getCombatantType() const override;
    virtual BattleDialogModelCombatant* clone() const override;
    virtual qreal getSizeFactor() const override;
    virtual int getSizeCategory() const override;
    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;
    virtual int getSkillModifier(Combatant::Skills skill) const override;
    virtual int getConditions() const override;
    virtual bool hasCondition(Combatant::Condition condition) const override;
    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual int getHitPoints() const override;
    virtual void setHitPoints(int hitPoints) override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

public slots:
    virtual void setConditions(int conditions) override;
    virtual void applyConditions(int conditions) override;
    virtual void removeConditions(int conditions) override;

protected:
    QUuid _combatantId;
    qreal _sizeFactor;
    int _conditions;
    int _speed;
};

#endif // BATTLEDIALOGMODELCOMBATANTDOWNLOAD_H
