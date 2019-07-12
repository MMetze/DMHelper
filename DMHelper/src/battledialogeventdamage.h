#ifndef BATTLEDIALOGEVENTDAMAGE_H
#define BATTLEDIALOGEVENTDAMAGE_H

#include "battledialogevent.h"
#include <QUuid>

class BattleDialogEventDamage : public BattleDialogEvent
{
    Q_OBJECT
public:
    explicit BattleDialogEventDamage(QUuid combatant, QUuid target, int damage);
    explicit BattleDialogEventDamage(const QDomElement& element);
    BattleDialogEventDamage(const BattleDialogEventDamage& other);
    virtual ~BattleDialogEventDamage() override;

    virtual int getType() const override;
    virtual void outputXML(QDomElement &element, bool isExport) override;

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
