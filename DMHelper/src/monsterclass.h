#ifndef MONSTERCLASS_H
#define MONSTERCLASS_H

#include "combatant.h"
#include "scaledpixmap.h"
#include "monsteraction.h"
#include <QString>
#include <QMap>

class QDomElement;

class MonsterClass : public QObject
{
    Q_OBJECT
public:

    explicit MonsterClass(const QString& name, QObject *parent = nullptr);
    explicit MonsterClass(const QDomElement &element, bool isImport, QObject *parent = nullptr);

    void inputXML(const QDomElement &element, bool isImport);
    QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;

    void beginBatchChanges();
    void endBatchChanges();

    int getType() const;
    bool getPrivate() const;

    bool getLegendary() const; // bool equivalent to having existing legendary actions
    QString getIcon() const;
    QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);
    QString getName() const;
    QString getMonsterType() const;
    QString getMonsterSubType() const;
    QString getMonsterSize() const;
    int getMonsterSizeCategory() const;
    qreal getMonsterSizeFactor() const;
    QString getSpeed() const;
    int getSpeedValue() const;
    QString getAlignment() const;
    QString getLanguages() const;
    int getArmorClass() const;
    Dice getHitDice() const;
    int getAverageHitPoints() const;
    QString getConditionImmunities() const;
    QString getDamageImmunities() const;
    QString getDamageResistances() const;
    QString getDamageVulnerabilities() const;
    QString getSenses() const;
    QString getChallenge() const;
    int getXP() const;
    int getStrength() const;
    int getDexterity() const;
    int getConstitution() const;
    int getIntelligence() const;
    int getWisdom() const;
    int getCharisma() const;

    int getAbilityValue(Combatant::Ability ability) const;
    int getSkillValue(Combatant::Skills skill) const;
    QString getSkillString() const;
    void setSkillString(const QString& skills);
    bool isSkillKnown(Combatant::Skills skill) const;

    QList<MonsterAction> getActions() const;
    void addAction(const MonsterAction& action);
    void setAction(int index, const MonsterAction& action);
    int removeAction(const MonsterAction& action);
    QList<MonsterAction> getLegendaryActions() const;
    void addLegendaryAction(const MonsterAction& action);
    void setLegendaryAction(int index, const MonsterAction& action);
    int removeLegendaryAction(const MonsterAction& action);
    QList<MonsterAction> getSpecialAbilities() const;
    void addSpecialAbility(const MonsterAction& action);
    void setSpecialAbility(int index, const MonsterAction& action);
    int removeSpecialAbility(const MonsterAction& action);
    QList<MonsterAction> getReactions() const;
    void addReaction(const MonsterAction& action);
    void setReaction(int index, const MonsterAction& action);
    int removeReaction(const MonsterAction& action);

    void cloneMonster(MonsterClass& other);

    static int convertSizeToCategory(const QString& monsterSize);
    static QString convertCategoryToSize(int category);
    static qreal convertSizeCategoryToScaleFactor(int category);
    static qreal convertSizeToScaleFactor(const QString& monsterSize);
    static void outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText);

public slots:
    void setPrivate(bool isPrivate);
    void setIcon(const QString& newIcon);
    void searchForIcon(const QString &newIcon);
    void clearIcon();
    void setName(const QString& name);
    void setMonsterType(const QString& monsterType);
    void setMonsterSubType(const QString& monsterSubType);
    void setMonsterSize(const QString& monsterSize);
    void setSpeed(const QString& speed);
    void setAlignment(const QString& alignment);
    void setLanguages(const QString& languages);
    void setArmorClass(int armorClass);
    void setHitDice(const Dice& hitDice);
    void setAverageHitPoints(int averageHitPoints);
    void setConditionImmunities(const QString& conditionImmunities);
    void setDamageImmunities(const QString& damageImmunities);
    void setDamageResistances(const QString& damageResistances);
    void setDamageVulnerabilities(const QString& damageVulnerabilities);
    void setSenses(const QString& senses);
    void setChallenge(const QString& challenge);
    void setStrength(int score);
    void setDexterity(int score);
    void setConstitution(int score);
    void setIntelligence(int score);
    void setWisdom(int score);
    void setCharisma(int score);

signals:
    void iconChanged();
    void dirty();

protected:

    void calculateHitDiceBonus();
    void registerChange();
    void checkForSkill(const QDomElement& element, const QString& skillName, Combatant::Skills skill, bool isImport);
    void readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList, bool isImport);
    void writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList, bool isExport) const;

    bool _private;
    // bool _legendary; Removed

    QString _icon;

    QString _name;
    QString _monsterType; // Now only type, no size
    QString _monsterSubType; // New

    QString _monsterSize; // new
    int _monsterSizeCategory;

    QString _speed;
    QString _alignment;
    QString _languages;
    int _armorClass;
    //QString _armorClassDescription; REMOVE
    Dice _hitDice;
    int _averageHitPoints;
    //QString _resistances; REMOVE
    QString _conditionImmunities; // New
    QString _damageImmunities; // New
    QString _damageResistances; // New
    QString _damageVulnerabilities; // New
    QString _senses;
    QString _challenge; // changed from float to string, need both getters

    //QString _skills; // completely changed to individual values to be read
    QMap<int,int> _skillValues;

    int _strength;
    int _dexterity;
    int _constitution;
    int _intelligence;
    int _wisdom;
    int _charisma;

    //int _XP; removed, derive from CR
    //QString _traits; removed
    //QString _actions; removed

    QList<MonsterAction> _actions; //NEW
    QList<MonsterAction> _legendaryActions; // NEW
    QList<MonsterAction> _specialAbilities; // NEW
    QList<MonsterAction> _reactions; // NEW

    bool _batchChanges;
    bool _changesMade;
    bool _iconChanged;

    ScaledPixmap _scaledPixmap;

public:

    static int getExperienceByCR(const QString& inputCR);
    static int getProficiencyByCR(const QString& inputCR);

};

#endif // MONSTERCLASS_H
