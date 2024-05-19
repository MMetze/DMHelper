#ifndef COMBATANT_H
#define COMBATANT_H

#include "campaignobjectbase.h"
#include "attack.h"
#include "dice.h"
#include "scaledpixmap.h"

class Combatant;

//typedef QPair<int, Combatant*> CombatantGroup;
//typedef QList<CombatantGroup> CombatantGroupList;

typedef QPair<int, int> ResourcePair;

class Combatant : public CampaignObjectBase
{
    Q_OBJECT
public:
    enum Ability
    {
        Ability_Strength = 0,
        Ability_Dexterity,
        Ability_Constitution,
        Ability_Intelligence,
        Ability_Wisdom,
        Ability_Charisma
    };

    enum Skills
    {
        Skills_strengthSave = 0,
        Skills_athletics,
        Skills_dexteritySave,
        Skills_stealth,
        Skills_acrobatics,
        Skills_sleightOfHand,
        Skills_constitutionSave,
        Skills_intelligenceSave,
        Skills_investigation,
        Skills_arcana,
        Skills_nature,
        Skills_history,
        Skills_religion,
        Skills_wisdomSave,
        Skills_medicine,
        Skills_animalHandling,
        Skills_perception,
        Skills_insight,
        Skills_survival,
        Skills_charismaSave,
        Skills_performance,
        Skills_deception,
        Skills_persuasion,
        Skills_intimidation,

        SKILLS_COUNT
    };

    const int SKILLS_UNSKILLED = 0;
    const int SKILLS_SKILLED = 1;
    const int SKILLS_EXPERT = 2;

    enum Condition
    {
        Condition_None          = 0x00000000,
        Condition_Blinded       = 0x00000001,
        Condition_Charmed       = 0x00000002,
        Condition_Deafened      = 0x00000004,
        Condition_Exhaustion_1  = 0x00000008,
        Condition_Exhaustion_2  = 0x00000010,
        Condition_Exhaustion_3  = 0x00000020,
        Condition_Exhaustion_4  = 0x00000040,
        Condition_Exhaustion_5  = 0x00000080,
        Condition_Frightened    = 0x00000100,
        Condition_Grappled      = 0x00000200,
        Condition_Incapacitated = 0x00000400,
        Condition_Invisible     = 0x00000800,
        Condition_Paralyzed     = 0x00001000,
        Condition_Petrified     = 0x00002000,
        Condition_Poisoned      = 0x00004000,
        Condition_Prone         = 0x00008000,
        Condition_Restrained    = 0x00010000,
        Condition_Stunned       = 0x00020000,
        Condition_Unconscious   = 0x00040000
    };

    enum Condition_Iterator
    {
        Condition_Iterator_None,
        Condition_Iterator_Blinded,
        Condition_Iterator_Charmed,
        Condition_Iterator_Deafened,
        Condition_Iterator_Exhaustion_1,
        Condition_Iterator_Exhaustion_2,
        Condition_Iterator_Exhaustion_3,
        Condition_Iterator_Exhaustion_4,
        Condition_Iterator_Exhaustion_5,
        Condition_Iterator_Frightened,
        Condition_Iterator_Grappled,
        Condition_Iterator_Incapacitated,
        Condition_Iterator_Invisible,
        Condition_Iterator_Paralyzed,
        Condition_Iterator_Petrified,
        Condition_Iterator_Poisoned,
        Condition_Iterator_Prone,
        Condition_Iterator_Restrained,
        Condition_Iterator_Stunned,
        Condition_Iterator_Unconscious,

        Condition_Iterator_Count
    };


    explicit Combatant(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Combatant() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;

    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    virtual Combatant* clone() const = 0;

    virtual int getCombatantType() const;
    virtual int getInitiative() const;
    virtual int getSpeed() const = 0;
    virtual int getArmorClass() const;
    virtual QList<Attack> getAttacks() const;
    virtual int getHitPoints() const;
    virtual Dice getHitDice() const;

    virtual QString getIconFile() const override;
    virtual QString getIconFileLocal() const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);

    virtual int getStrength() const = 0;
    virtual int getDexterity() const = 0;
    virtual int getConstitution() const = 0;
    virtual int getIntelligence() const = 0;
    virtual int getWisdom() const = 0;
    virtual int getCharisma() const = 0;

    virtual int getAbilityValue(Ability ability) const;
    static int getAbilityMod(int ability);
    static QString getAbilityModStr(int ability);
    static QString convertModToStr(int modifier);
    static Ability getSkillAbility(Skills skill);
    static bool isSkillSavingThrow(Skills skill);
//    static QList<Combatant*> instantiateCombatants(CombatantGroup combatantGroup);

    static int getConditionCount();
    static Condition getConditionByIndex(int index);
    static QString getConditionIcon(int condition);
    static QString getConditionTitle(int condition);
    static QString getConditionDescription(int condition);
    static void drawConditions(QPaintDevice* target, int conditions);
    static QStringList getConditionString(int conditions);

    virtual int getConditions() const;
    virtual bool hasCondition(Condition condition) const;

signals:

public slots:
    virtual void setInitiative(int initiative);
    virtual void setArmorClass(int armorClass);
    virtual void addAttack(const Attack& attack);
    virtual void removeAttack(int index);
    virtual void setHitPoints(int hitPoints);
    virtual void applyDamage(int damage);
    virtual void setHitDice(const Dice& hitDice);
    virtual void setConditions(int conditions);
    virtual void applyConditions(int conditions);
    virtual void removeConditions(int conditions);
    virtual void addCondition(Condition condition);
    virtual void removeCondition(Condition condition);
    virtual void clearConditions();
    virtual void setIcon(const QString &newIcon);

protected:

    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    void registerChange();

    int _initiative;
    int _armorClass;
    QList<Attack> _attacks;
    int _hitPoints;
    Dice _hitDice;
    int _conditions;
    QString _icon;
    ScaledPixmap _iconPixmap;

    bool _batchChanges;
    bool _changesMade;

};

Q_DECLARE_METATYPE(Combatant*)

#endif // COMBATANT_H
