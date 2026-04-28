#ifndef COMBATANTTEMPLATEADAPTER_H
#define COMBATANTTEMPLATEADAPTER_H

#include "templateobject.h"
#include <QObject>
#include <QHash>
#include <QVariant>

class BattleDialogModelCombatant;
class BattleDialogModelMonsterBase;
class BattleDialogModelCharacter;
class MonsterClassv2;
class Characterv2;

// Adapter that exposes a BattleDialogModelCombatant (plus its underlying
// MonsterClassv2 or Characterv2) as a single TemplateObject so that the
// template binding pipeline (TemplateFactory::readObjectData / populateWidget)
// can drive a combatant initiative widget from a .ui template.
//
// "Model" keys (initiative, moved, isShown, isKnown, isDone, hp, ac, name,
// conditions, perRoundResources) are routed to the combatant model. All other
// keys are forwarded to the underlying class/character TemplateObject.
class CombatantTemplateAdapter : public QObject, public TemplateObject
{
    Q_OBJECT
public:
    explicit CombatantTemplateAdapter(BattleDialogModelCombatant* combatant, QObject* parent = nullptr);
    virtual ~CombatantTemplateAdapter() override;

    BattleDialogModelCombatant* getCombatant() const;
    TemplateObject* getInner() const;

    // Model-side keys exposed by the adapter.
    static const char* KEY_NAME;
    static const char* KEY_INITIATIVE;
    static const char* KEY_MOVED;
    static const char* KEY_IS_SHOWN;
    static const char* KEY_IS_KNOWN;
    static const char* KEY_IS_DONE;
    static const char* KEY_HP;
    static const char* KEY_AC;
    static const char* KEY_CONDITIONS;
    static const char* KEY_PER_ROUND_RESOURCES;

    // Per-list-entry keys for conditions / perRoundResources.
    static const char* CONDITION_KEY_ID;
    static const char* RESOURCE_KEY_NAME;
    static const char* RESOURCE_KEY_MAX;
    static const char* RESOURCE_KEY_CURRENT;
    static const char* RESOURCE_KEY_RECHARGE;

    // From TemplateObject
    virtual bool hasValue(const QString& key) const override;
    virtual QString getValueAsString(const QString& key) const override;
    virtual QString getStringValue(const QString& key) const override;
    virtual int getIntValue(const QString& key) const override;
    virtual bool getBoolValue(const QString& key) const override;
    virtual QList<QVariant> getListValue(const QString& key) const override;

    virtual void setValue(const QString& key, const QVariant& value) override;
    virtual void setValue(const QString& key, const QString& value) override;
    virtual void setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue) override;
    virtual void setListValue(const QString& key, int index, const QString& listEntryKey, const QString& listEntryValue) override;

protected:
    virtual QHash<QString, QVariant>* valueHash() override;
    virtual const QHash<QString, QVariant>* valueHash() const override;
    virtual void declareDirty() override;

private:
    bool isModelKey(const QString& key) const;
    BattleDialogModelMonsterBase* monsterBase() const;
    BattleDialogModelCharacter* characterModel() const;
    MonsterClassv2* monsterClass() const;
    Characterv2* character() const;

    BattleDialogModelCombatant* _combatant;
    mutable QHash<QString, QVariant> _dummyHash;
};

#endif // COMBATANTTEMPLATEADAPTER_H
