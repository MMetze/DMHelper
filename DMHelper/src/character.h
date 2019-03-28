#ifndef CHARACTER_H
#define CHARACTER_H

#include "combatant.h"
#include <QString>
#include <QVector>
#include <QPair>

class QDomElement;

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

        INTVALUE_COUNT
    };

    explicit Character(QObject *parent = nullptr);
    explicit Character(QDomElement &element, QObject *parent = nullptr);
    explicit Character(const Character &obj);  // copy constructor

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // From Combatant
    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    virtual Combatant* clone() const;

    virtual int getType() const;
    virtual int getDndBeyondID() const;
    virtual void setDndBeyondID(int id);

    virtual int getSpeed() const;

    virtual int getStrength() const;
    virtual int getDexterity() const;
    virtual int getConstitution() const;
    virtual int getIntelligence() const;
    virtual int getWisdom() const;
    virtual int getCharisma() const;

    QString getStringValue(StringValue key) const;
    int getIntValue(IntValue key) const;
    bool getSkillValue(Skills key) const;

    void setStringValue(StringValue key, const QString& value);
    void setIntValue(IntValue key, int value);
    void setSkillValue(Skills key, bool value);

    bool getActive() const;
    void setActive(bool active);

    int getTotalLevel() const;
    int getXPThreshold(int threshold) const;
    int getNextLevelXP() const;
    int getProficiencyBonus() const;
    int getPassivePerception() const;

    static int findKeyForSkillName(const QString& skillName);
    static QString getWrittenSkillName(int skill);

signals:
    void iconChanged();

public slots:
    virtual void setIcon(const QString &newIcon);

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

private:
    void setDefaultValues();

    int _dndBeyondID;
    QVector<QString> _stringValues;
    QVector<int> _intValues;
    QVector<bool> _skillValues;

    bool _active;
    bool _iconChanged;
};

#endif // CHARACTER_H
