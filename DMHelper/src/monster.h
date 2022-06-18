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
    explicit Monster(MonsterClass* monsterClass, const QString& name = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    // From Combatant
    virtual void beginBatchChanges() override;
    virtual void endBatchChanges() override;

    virtual Combatant* clone() const override;

    virtual int getCombatantType() const override;
    virtual int getSpeed() const override;
    virtual int getArmorClass() const override;
    virtual Dice getHitDice() const override;

    MonsterClass* getMonsterClass() const;
    QString getMonsterClassName() const;
    virtual QString getIcon(bool localOnly) const override;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    int getPassivePerception() const;
    bool getActive() const;
    QString getNotes() const;

public slots:
    virtual void setMonsterClass(MonsterClass* newMonsterClass);
    virtual void setIcon(const QString& newIcon) override;
    void setPassivePerception(int value);
    void setActive(bool value);
    void setNotes(const QString& newNotes);

signals:
    void iconChanged();

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    MonsterClass* _monsterClass;
    int _passivePerception;
    bool _active;
    QString _notes;

    bool _iconChanged;

};

#endif // MONSTER_H
