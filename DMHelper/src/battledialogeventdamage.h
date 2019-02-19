#ifndef BATTLEDIALOGEVENTDAMAGE_H
#define BATTLEDIALOGEVENTDAMAGE_H

#include "battledialogevent.h"

class BattleDialogEventDamage : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventDamage(int combatant, int target, int damage);
    BattleDialogEventDamage(const QDomElement& element);
    BattleDialogEventDamage(const BattleDialogEventDamage& other);
    virtual ~BattleDialogEventDamage();

    virtual int getType() const;
    virtual void outputXML(QDomElement &element);

    virtual int getCombatant() const;
    virtual int getTarget() const;
    virtual int getDamage() const;

signals:

public slots:

protected:
    int _combatant;
    int _target;
    int _damage;
};

#endif // BATTLEDIALOGEVENTDAMAGE_H
