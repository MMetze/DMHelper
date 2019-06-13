#ifndef BATTLEDIALOGMODELCOMBATANT_H
#define BATTLEDIALOGMODELCOMBATANT_H

#include "campaignobjectbase.h"
#include "scaledpixmap.h"
#include "combatant.h"
#include <QPoint>

class BattleDialogModelCombatant : public CampaignObjectBase
{
    Q_OBJECT

public:
    BattleDialogModelCombatant();
    BattleDialogModelCombatant(Combatant* combatant);
    BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position);
    BattleDialogModelCombatant(const BattleDialogModelCombatant& other);
    virtual ~BattleDialogModelCombatant();

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);

    // Local
    virtual int getType() const = 0;
    virtual BattleDialogModelCombatant* clone() const = 0;

    virtual bool getShown() const;
    virtual bool getKnown() const;

    int getInitiative() const;
    void setInitiative(int initiative);

    const QPointF& getPosition() const;
    void setPosition(const QPointF& position);
    virtual int getSizeFactor() const = 0;
    virtual int getSizeCategory() const = 0;

    Combatant* getCombatant() const;

    virtual int getStrength() const = 0;
    virtual int getDexterity() const = 0;
    virtual int getConstitution() const = 0;
    virtual int getIntelligence() const = 0;
    virtual int getWisdom() const = 0;
    virtual int getCharisma() const = 0;
    virtual int getAbilityValue(Combatant::Ability ability) const;
    virtual int getSkillModifier(Combatant::Skills skill) const = 0;

    virtual int getSpeed() const = 0;
    virtual int getArmorClass() const = 0;
    virtual int getHitPoints() const = 0;
    virtual void setHitPoints(int hitPoints) = 0;
    virtual QString getName() const = 0;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const = 0;

signals:
    void combatantMoved(BattleDialogModelCombatant* combatant);

protected:
    // From BattleDialogModelCombatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) = 0;

    void setCombatant(Combatant* combatant);

    Combatant* _combatant;
    int _initiative;
    QPointF _position;
};

#endif // BATTLEDIALOGMODELCOMBATANT_H
