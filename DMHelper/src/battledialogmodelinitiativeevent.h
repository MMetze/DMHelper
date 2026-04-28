#ifndef BATTLEDIALOGMODELINITIATIVEEVENT_H
#define BATTLEDIALOGMODELINITIATIVEEVENT_H

#include "battledialogmodelcombatant.h"

// Synthetic combatant representing a scheduled event in the initiative order
// (e.g. lair actions, environmental effects, hazards). Holds a name and an
// initiative count; carries no statblock, no token, no map presence. The
// "active" toggle on the InitiativeEventWidget maps to getKnown()/setKnown()
// from the base class — when unknown, the event is skipped by
// BattleFrame::getNextCombatant().
class BattleDialogModelInitiativeEvent : public BattleDialogModelCombatant
{
    Q_OBJECT
public:
    explicit BattleDialogModelInitiativeEvent(const QString& name = QString(), int initiative = 0, QObject* parent = nullptr);
    virtual ~BattleDialogModelInitiativeEvent() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement& element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    // From BattleDialogModelCombatant
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
    virtual QStringList getConditionList() const override;
    virtual bool hasConditionId(const QString& conditionId) const override;

    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual int getHitPoints() const override;
    virtual void setHitPoints(int hitPoints) override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const override;

public slots:
    virtual void setConditionList(const QStringList& conditions) override;
    virtual void addConditionId(const QString& conditionId) override;
    virtual void removeConditionId(const QString& conditionId) override;
    virtual void clearConditions() override;

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument& doc) override;
    virtual void internalOutputXML(QDomDocument& doc, QDomElement& element, QDir& targetDirectory, bool isExport) override;
};

#endif // BATTLEDIALOGMODELINITIATIVEEVENT_H
