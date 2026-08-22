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

    // Per-list-entry keys for conditions / perRoundResources.
    static const char* CONDITION_KEY_ID;
    static const char* RESOURCE_KEY_NAME;
    static const char* RESOURCE_KEY_MAX;
    static const char* RESOURCE_KEY_CURRENT;
    static const char* RESOURCE_KEY_RECHARGE;

    // Translate a legacy key (e.g. "hitPoints", "armorClass", "initiative") to
    // its canonical dmh:-prefixed form. Returns the input unchanged if no
    // alias is registered. Exposed for tests and for callers that want to
    // pre-normalize keys before storing them.
    static QString canonicalKey(const QString& key);

    // Legacy alias table for XML-level compatibility conversion only.
    // Maps pre-v3 unprefixed attribute names ("hitPoints", "initiative",
    // etc.) to their canonical dmh:-prefixed equivalents. NOT used at
    // runtime — see canonicalKey() comment in the .cpp.
    static const QHash<QString, QString>& legacyAliasTable();

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

private slots:
    // Forwarders that publish model-side change signals through this
    // adapter's notifier so the template-binding pipeline can update bound
    // widgets incrementally instead of rebuilding the whole binding tree.
    void onCombatantInitiativeChanged();
    void onCombatantConditionsChanged();
    void onCombatantMoveUpdated();
    void onCombatantVisibilityChanged();
    void onCombatantDoneChanged();
    void onCombatantOverrideChanged(BattleDialogModelCombatant* combatant, const QString& key);
    void onMonsterDataChanged();
    void onMonsterResourceCountChanged(BattleDialogModelMonsterBase* monster, const QString& resourceName, int newValue);
    void onInnerValueChanged(const QString& innerKey);

private:
    void wireModelToNotifier();
    bool isModelKey(const QString& key) const;
    bool isResourceKey(const QString& key) const;
    QString resourceStorageString(const QString& key) const;
    QString normalizeResourceStorageString(const QString& key, const QString& value) const;
    BattleDialogModelMonsterBase* monsterBase() const;
    BattleDialogModelCharacter* characterModel() const;
    MonsterClassv2* monsterClass() const;
    Characterv2* character() const;

    BattleDialogModelCombatant* _combatant;
    mutable QHash<QString, QVariant> _dummyHash;
};

#endif // COMBATANTTEMPLATEADAPTER_H
