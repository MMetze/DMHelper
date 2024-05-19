#ifndef BATTLEDIALOGMODELCHARACTER_H
#define BATTLEDIALOGMODELCHARACTER_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include <QString>
#include <QPoint>

class Characterv2;

class BattleDialogModelCharacter : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    BattleDialogModelCharacter(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelCharacter(Characterv2* character);
    explicit BattleDialogModelCharacter(Characterv2* character, int initiative, const QPointF& position);
    virtual ~BattleDialogModelCharacter() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    // Local
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
    virtual QString getName() const override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

    Characterv2* getCharacter() const;
    void setCharacter(Characterv2* character);

public slots:
    virtual void setConditions(int conditions) override;
    virtual void applyConditions(int conditions) override;
    virtual void removeConditions(int conditions) override;
    virtual void clearConditions() override;

protected:

};
#endif // BATTLEDIALOGMODELCHARACTER_H
