#ifndef BATTLEDIALOGEVENTDAMAGE_H
#define BATTLEDIALOGEVENTDAMAGE_H

#include "battledialogevent.h"
#include <QUuid>

class BattleDialogEventDamage : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventDamage(QUuid combatant, QUuid target, int damage);
    BattleDialogEventDamage(const QDomElement& element);
    BattleDialogEventDamage(const BattleDialogEventDamage& other);
    virtual ~BattleDialogEventDamage();

    virtual int getType() const;
    virtual void outputXML(QDomElement &element, bool isExport);

    virtual QUuid getCombatant() const;
    virtual QUuid getTarget() const;
    virtual int getDamage() const;

signals:

public slots:

protected:
    QUuid _combatant;
    QUuid _target;
    int _damage;
};

#endif // BATTLEDIALOGEVENTDAMAGE_H
