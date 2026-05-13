#ifndef COMBATANT_H
#define COMBATANT_H

#include "campaignobjectbase.h"
#include "attack.h"
#include "dice.h"
#include "scaledpixmap.h"
#include <QStringList>

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
    virtual QColor getBackgroundColor() const;

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

    // String-based condition API
    QStringList getConditionList() const;
    bool hasConditionId(const QString& conditionId) const;

signals:

public slots:
    virtual void setInitiative(int initiative);
    virtual void setArmorClass(int armorClass);
    virtual void addAttack(const Attack& attack);
    virtual void removeAttack(int index);
    virtual void setHitPoints(int hitPoints);
    virtual void applyDamage(int damage);
    virtual void setHitDice(const Dice& hitDice);
    virtual void setConditionList(const QStringList& conditions);
    virtual void addConditionId(const QString& conditionId);
    virtual void removeConditionId(const QString& conditionId);
    virtual void clearConditions();
    virtual void setIcon(const QString &newIcon);
    virtual void setBackgroundColor(const QColor &color);

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
    QStringList _conditionList;
    QString _icon;
    ScaledPixmap _iconPixmap;
    QColor _backgroundColor;

    bool _batchChanges;
    bool _changesMade;

};

Q_DECLARE_METATYPE(Combatant*)

#endif // COMBATANT_H
