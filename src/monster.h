#ifndef MONSTER_H
#define MONSTER_H

#include "combatant.h"
#include <QString>

class QDomElement;
class MonsterClass;

class Monster : public Combatant
{
    Q_OBJECT
public:
    explicit Monster(MonsterClass* monsterClass, QObject *parent = 0);
    explicit Monster(MonsterClass* monsterClass, const QDomElement &element, QObject *parent = 0);
    explicit Monster(const Monster &obj);  // copy constructor

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // From Combatant
    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    virtual Combatant* clone() const;

    virtual int getType() const;
    virtual int getSpeed() const;
    virtual int getArmorClass() const;
    virtual Dice getHitDice() const;

    MonsterClass* getMonsterClass() const;
    QString getMonsterClassName() const;
    virtual QString getIcon(bool localOnly) const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);

    virtual int getStrength() const;
    virtual int getDexterity() const;
    virtual int getConstitution() const;
    virtual int getIntelligence() const;
    virtual int getWisdom() const;
    virtual int getCharisma() const;

    int getPassivePerception() const;
    bool getActive() const;
    QString getNotes() const;

public slots:
    virtual void setMonsterClass(MonsterClass* newMonsterClass);
    virtual void setIcon(const QString& newIcon);
    void setPassivePerception(int value);
    void setActive(bool value);
    void setNotes(const QString& newNotes);

signals:
    void iconChanged();

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

    MonsterClass* _monsterClass;
    int _passivePerception;
    bool _active;
    QString _notes;

    bool _iconChanged;

};

#endif // MONSTER_H
