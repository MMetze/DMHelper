#ifndef COMBATANTDOWNLOAD_H
#define COMBATANTDOWNLOAD_H

#include "combatant.h"

class CombatantDownload : public Combatant
{
    Q_OBJECT
public:
    explicit CombatantDownload(QObject *parent = nullptr);
    virtual ~CombatantDownload() override;

    virtual Combatant* clone() const override;

    virtual int getSpeed() const override;
    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    virtual qreal getSizeFactor() const;

protected:

    qreal _sizeFactor;
};

#endif // COMBATANTDOWNLOAD_H
