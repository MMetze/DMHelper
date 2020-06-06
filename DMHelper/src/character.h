#ifndef CHARACTER_H
#define CHARACTER_H

#include "combatant.h"
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
        StringValue_spells,
        StringValue_notes,
        StringValue_size,

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
        IntValue_experience,
        IntValue_jackofalltrades,

        INTVALUE_COUNT
    };

    explicit Character(const QString& name = QString(), QObject *parent = nullptr);
    //explicit Character(QDomElement &element, bool isImport, QObject *parent = nullptr);
    //explicit Character(const Character &obj);  // copy constructor

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

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

    void setStringValue(StringValue key, const QString& value);
    void setIntValue(IntValue key, int value);
    void setSkillValue(Skills key, bool value);
    void setSkillValue(Skills key, int value);
    void setSkillExpertise(Skills key, bool value);

    bool getActive() const;
    void setActive(bool active);

    int getTotalLevel() const;
    int getXPThreshold(int threshold) const;
    int getNextLevelXP() const;
    int getProficiencyBonus() const;
    int getPassivePerception() const;

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

private:
    void setDefaultValues();

    int _dndBeyondID;
    QVector<QString> _stringValues;
    QVector<int> _intValues;
    QVector<int> _skillValues;

    bool _active;
    bool _iconChanged;
};

#endif // CHARACTER_H
