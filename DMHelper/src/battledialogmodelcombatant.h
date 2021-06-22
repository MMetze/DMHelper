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
    BattleDialogModelCombatant(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelCombatant(Combatant* combatant);
    explicit BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position);
    //BattleDialogModelCombatant(const BattleDialogModelCombatant& other);
    virtual ~BattleDialogModelCombatant() override;

    // From CampaignObjectBase
    //virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual QDomElement outputNetworkXML(QDomDocument &doc) override;

    // Local
    virtual int getCombatantType() const = 0;
    virtual BattleDialogModelCombatant* clone() const = 0;

    virtual bool getShown() const;
    virtual bool getKnown() const;
    virtual bool getSelected() const;

    int getInitiative() const;
    void setInitiative(int initiative);

    const QPointF& getPosition() const;
    void setPosition(const QPointF& position);
    virtual qreal getSizeFactor() const = 0;
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
    virtual int getConditions() const = 0;
    virtual bool hasCondition(Combatant::Condition condition) const = 0;

    virtual int getSpeed() const = 0;
    virtual int getArmorClass() const = 0;
    virtual int getHitPoints() const = 0;
    virtual void setHitPoints(int hitPoints) = 0;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const = 0;

    qreal getMoved();
    void setMoved(qreal moved);
    void incrementMoved(qreal moved);
    void resetMoved();

public slots:
    virtual void setShown(bool isShown);
    virtual void setKnown(bool isKnown);
    virtual void setSelected(bool isSelected);
    virtual void setConditions(int conditions) = 0;
    virtual void applyConditions(int conditions) = 0;
    virtual void removeConditions(int conditions) = 0;

signals:
    void initiativeChanged(BattleDialogModelCombatant* combatant);
    void combatantMoved(BattleDialogModelCombatant* combatant);
    void combatantSelected(BattleDialogModelCombatant* combatant);
    void moveUpdated();

protected:
    // From BattleDialogModelCombatant
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    void setCombatant(Combatant* combatant);
    void copyValues(const BattleDialogModelCombatant &other);

    Combatant* _combatant;
    int _initiative;
    QPointF _position;
    qreal _moved;
    bool _isShown;
    bool _isKnown;
    bool _isSelected;
};

#endif // BATTLEDIALOGMODELCOMBATANT_H
