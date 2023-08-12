#ifndef CHARACTER_H
#define CHARACTER_H

#include "combatant.h"
#include "monsteraction.h"
#include <QString>
#include <QVector>
#include <QPair>

class QDomElement;
class MonsterClass;

typedef QPair<int,int> AbilitySkillPair;
Q_DECLARE_METATYPE(AbilitySkillPair);

class Character : public Combatant
{
    Q_OBJECT
public:

    enum StringValue
    {
        StringValue_player = 0,
        StringValue_race,
        StringValue_subrace,
        StringValue_sex,
        StringValue_alignment,
        StringValue_background,
        StringValue_class,
        StringValue_class2,
        StringValue_class3,
        StringValue_age,
        StringValue_height,
        StringValue_weight,
        StringValue_eyes,
        StringValue_hair,
        StringValue_equipment,
        StringValue_proficiencies,
        StringValue_notes,
        StringValue_size,
        StringValue_experience,

        STRINGVALUE_COUNT
    };

    enum IntValue
    {
        IntValue_level = 0,
        IntValue_level2,
        IntValue_level3,
        IntValue_strength,
        IntValue_dexterity,
        IntValue_constitution,
        IntValue_intelligence,
        IntValue_wisdom,
        IntValue_charisma,
        IntValue_speed,
        IntValue_platinum,
        IntValue_gold,
        IntValue_silver,
        IntValue_copper,
        IntValue_jackofalltrades,
        IntValue_maximumHP,
        IntValue_pactMagicSlots,
        IntValue_pactMagicUsed,
        IntValue_pactMagicLevel,
        IntValue_cantrips,

        INTVALUE_COUNT
    };

    explicit Character(const QString& name = QString(), QObject *parent = nullptr);
    //explicit Character(QDomElement &element, bool isImport, QObject *parent = nullptr);
    //explicit Character(const Character &obj);  // copy constructor

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    // From Combatant
    virtual void beginBatchChanges() override;
    virtual void endBatchChanges() override;

    virtual Combatant* clone() const override;

    virtual int getCombatantType() const override;
    virtual int getDndBeyondID() const;
    virtual void setDndBeyondID(int id);

    virtual bool isInParty() const;

    virtual int getSpeed() const override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    QString getStringValue(StringValue key) const;
    int getIntValue(IntValue key) const;
    bool getSkillValue(Skills key) const;
    bool getSkillExpertise(Skills key) const;
    int getSkillBonus(Skills key) const;

    void setStringValue(StringValue key, const QString& value);
    void setIntValue(IntValue key, int value);
    void setSkillValue(Skills key, bool value);
    void setSkillValue(Skills key, int value);
    void setSkillExpertise(Skills key, bool value);

    int spellSlotLevels() const;
    QVector<int> getSpellSlots() const;
    QVector<int> getSpellSlotsUsed() const;
    void setSpellSlots(int level, int slotCount);
    int getSpellSlots(int level);
    void setSpellSlotsUsed(int level, int slotsUsed);
    int getSpellSlotsUsed(int level);
    void clearSpellSlotsUsed();

    QString getSpellString();
    void setSpellString(const QString& spellString);

    bool getActive() const;
    void setActive(bool active);

    int getTotalLevel() const;
    int getXPThreshold(int threshold) const;
    int getNextLevelXP() const;
    int getProficiencyBonus() const;
    int getPassivePerception() const;

    QList<MonsterAction> getActions() const;
    void addAction(const MonsterAction& action);
    void setAction(int index, const MonsterAction& action);
    int removeAction(const MonsterAction& action);

    virtual void copyMonsterValues(MonsterClass& monster);

    static int findKeyForSkillName(const QString& skillName);
    static QString getWrittenSkillName(int skill);

signals:
    void iconChanged();

public slots:
    virtual void setIcon(const QString &newIcon) override;

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

private:
    void setDefaultValues();
    void readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList, bool isImport);
    void writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList, bool isExport) const;

    int _dndBeyondID;
    QVector<QString> _stringValues;
    QVector<int> _intValues;
    QVector<int> _skillValues;
    QVector<int> _spellSlots;
    QVector<int> _spellSlotsUsed;
    QString _spellList;

    QList<MonsterAction> _actions;

    bool _active;
    bool _iconChanged;
};

#endif // CHARACTER_H
