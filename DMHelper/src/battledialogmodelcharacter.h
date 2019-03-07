#ifndef BATTLEDIALOGMODELCHARACTER_H
#define BATTLEDIALOGMODELCHARACTER_H

#include "battledialogmodelcombatant.h"
#include "combatant.h"
#include <QString>
#include <QPoint>

class Character;

class BattleDialogModelCharacter : public BattleDialogModelCombatant
{
    Q_OBJECT

public:
    BattleDialogModelCharacter();
    BattleDialogModelCharacter(Character* character);
    BattleDialogModelCharacter(Character* character, int initiative, const QPointF& position);
    BattleDialogModelCharacter(const BattleDialogModelCharacter& other);
    virtual ~BattleDialogModelCharacter();

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // Local
    virtual int getType() const;
    virtual BattleDialogModelCombatant* clone() const;
    virtual int getSizeFactor() const;
    virtual int getSizeCategory() const;

    virtual int getStrength() const;
    virtual int getDexterity() const;
    virtual int getConstitution() const;
    virtual int getIntelligence() const;
    virtual int getWisdom() const;
    virtual int getCharisma() const;
    virtual int getSkillModifier(Combatant::Skills skill) const;

    virtual int getSpeed() const;
    virtual int getArmorClass() const;
    virtual int getHitPoints() const;
    virtual void setHitPoints(int hitPoints);
    virtual QString getName() const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const;

    Character* getCharacter() const;
    void setCharacter(Character* character);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

};
#endif // BATTLEDIALOGMODELCHARACTER_H
