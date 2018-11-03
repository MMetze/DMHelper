#ifndef MONSTERCLASS_H
#define MONSTERCLASS_H

#include "combatant.h"
#include "scaledpixmap.h"
#include <QString>
#include <QMap>

class QDomElement;

class MonsterClass : public QObject
{
    Q_OBJECT
public:
    explicit MonsterClass(const QString& name, QObject *parent = 0);
    explicit MonsterClass(QDomElement &element, QObject *parent = 0);

    void inputXML(const QDomElement &element);
    void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory) const;

    void beginBatchChanges();
    void endBatchChanges();

    int getType() const;
    bool getPrivate() const;

    bool getLegendary() const;
    QString getIcon() const;
    QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);
    QString getName() const;
    QString getMonsterType() const;
    QString getMonsterSize() const;
    int getMonsterSizeCategory() const;
    int getMonsterSizeFactor() const;
    QString getAlignment() const;
    int getArmorClass() const;
    QString getArmorClassDescription() const;
    Dice getHitDice() const;
    int getAverageHitPoints() const;
    QString getSpeed() const;
    int getStrength() const;
    int getDexterity() const;
    int getConstitution() const;
    int getIntelligence() const;
    int getWisdom() const;
    int getCharisma() const;
    QString getSkills() const;
    QString getResistances() const;
    QString getSenses() const;
    QString getLanguages() const;
    float getChallenge() const;
    int getXP() const;
    QString getTraits() const;
    QString getActions() const;

    int getAbilityValue(Combatant::Ability ability) const;
    int getSkillValue(Combatant::Skills skill) const;

    static int convertSizeToCategory(const QString& monsterSize);
    static int convertSizeCategoryToScaleFactor(int category);
    static int convertSizeToScaleFactor(const QString& monsterSize);


public slots:
    void setPrivate(bool isPrivate);
    void setLegendary(bool isLegendary);
    void setIcon(const QString& newIcon);
    void searchForIcon(const QString &newIcon);
    void setName(const QString& name);
    void setMonsterType(const QString& monsterType);
    void setAlignment(const QString& alignment);
    void setArmorClass(int armorClass);
    void setArmorClassDescription(const QString& armorClassDescription);
    void setHitDice(const Dice& hitDice);
    void setAverageHitPoints(int averageHitPoints);
    void setSpeed(const QString& speed);
    void setStrength(int score);
    void setDexterity(int score);
    void setConstitution(int score);
    void setIntelligence(int score);
    void setWisdom(int score);
    void setCharisma(int score);
    void setSkills(const QString& skills);
    void setResistances(const QString& resistances);
    void setSenses(const QString& senses);
    void setLanguages(const QString& languages);
    void setChallenge(float challenge);
    void setXP(int xp);
    void setTraits(const QString& traits);
    void setActions(const QString& actions);

signals:
    void iconChanged();
    void dirty();

protected:

    void registerChange();

    bool _private;
    bool _legendary;
    QString _icon;
    QString _name;
    QString _monsterType;
    int _monsterSizeCategory;
    QString _alignment;
    int _armorClass;
    QString _armorClassDescription;
    Dice _hitDice;
    int _averageHitPoints;
    QString _speed;
    int _strength;
    int _dexterity;
    int _constitution;
    int _intelligence;
    int _wisdom;
    int _charisma;
    QString _skills;
    QMap<int,int> _skillValues;
    QString _resistances;
    QString _senses;
    QString _languages;
    float _challenge;
    int _XP;
    QString _traits;
    QString _actions;

    bool _batchChanges;
    bool _changesMade;
    bool _iconChanged;

    ScaledPixmap _scaledPixmap;

};

#endif // MONSTERCLASS_H
