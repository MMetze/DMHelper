#ifndef BATTLEDIALOGMODELCOMBATANT_H
#define BATTLEDIALOGMODELCOMBATANT_H

#include "battledialogmodelobject.h"
#include "scaledpixmap.h"
#include "combatant.h"
#include <QPoint>
#include <QUuid>
#include <QHash>
#include <QStringList>
#include <QVariant>


class BattleDialogModelCombatant : public BattleDialogModelObject
{
    Q_OBJECT

public:
    BattleDialogModelCombatant(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelCombatant(Combatant* combatant);
    explicit BattleDialogModelCombatant(Combatant* combatant, int initiative, const QPointF& position);
    virtual ~BattleDialogModelCombatant() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;

    // Local
    virtual int getCombatantType() const = 0;
    virtual BattleDialogModelCombatant* clone() const = 0;

    virtual bool getShown() const;
    virtual bool getKnown() const;
    virtual bool getSelected() const;
    virtual bool getDone() const;

    int getInitiative() const;
    void setInitiative(int initiative);

    int getSortPosition() const;
    void setSortPosition(int sortPosition);

    QUuid getGroupId() const;
    void setGroupId(const QUuid& groupId);

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
    virtual QStringList getConditionList() const = 0;
    virtual bool hasConditionId(const QString& conditionId) const = 0;

    virtual int getSpeed() const = 0;
    virtual int getArmorClass() const = 0;
    virtual int getHitPoints() const = 0;
    virtual void setHitPoints(int hitPoints) = 0;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) const = 0;

    qreal getMoved();
    void setMoved(qreal moved);
    void incrementMoved(qreal moved);
    void resetMoved();

    // Generic per-encounter override bag. Stores values for canonical "dmh:"-prefixed
    // keys (and any ruleset-defined overrides) without imposing a type or XML schema.
    // Phase 1: storage and access only; existing typed accessors are not yet wired in.
    QVariant    getOverride(const QString& key) const;
    bool        hasOverride(const QString& key) const;
    void        setOverride(const QString& key, const QVariant& value);
    void        clearOverride(const QString& key);
    QStringList overrideKeys() const;

    // Canonical key names. The dmh: prefix is reserved for engine-owned
    // attributes so they can never collide with a ruleset-defined key.
    static const char* DMH_KEY_NAME;
    static const char* DMH_KEY_INITIATIVE;
    static const char* DMH_KEY_MOVED;
    static const char* DMH_KEY_IS_SHOWN;
    static const char* DMH_KEY_IS_KNOWN;
    static const char* DMH_KEY_IS_DONE;
    static const char* DMH_KEY_HEALTH;
    static const char* DMH_KEY_CONDITIONS;
    static const char* DMH_KEY_PER_ROUND_RESOURCES;

public slots:
    virtual void setShown(bool isShown);
    virtual void setKnown(bool isKnown);
    virtual void setSelected(bool isSelected) override;
    virtual void setDone(bool isDone);
    virtual void setConditionList(const QStringList& conditions) = 0;
    virtual void addConditionId(const QString& conditionId) = 0;
    virtual void removeConditionId(const QString& conditionId) = 0;
    virtual void clearConditions() = 0;

signals:
    void initiativeChanged(BattleDialogModelCombatant* combatant);
    void conditionsChanged(BattleDialogModelCombatant* combatant);
    void combatantSelected(BattleDialogModelCombatant* combatant);
    void moveUpdated();
    void visibilityChanged();
    void combatantDoneChanged(BattleDialogModelCombatant* combatant);
    void overrideChanged(BattleDialogModelCombatant* combatant, const QString& key);

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    // Local
    void setCombatant(Combatant* combatant);

    Combatant* _combatant;
    int _initiative;
    int _sortPosition;
    QUuid _groupId;
    qreal _moved;
    bool _isShown;
    bool _isKnown;
    bool _isSelected;
    bool _isDone;
    QHash<QString, QVariant> _overrides;
};

#endif // BATTLEDIALOGMODELCOMBATANT_H
