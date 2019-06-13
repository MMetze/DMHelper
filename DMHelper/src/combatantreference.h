#ifndef COMBATANTREFERENCE_H
#define COMBATANTREFERENCE_H

#include "campaignobjectbase.h"
#include "combatant.h"
#include <QUuid>

class CombatantReference : public Combatant
{
    Q_OBJECT
public:
    explicit CombatantReference(QObject *parent = nullptr);
    explicit CombatantReference(const Combatant &combatant, QObject *parent = nullptr);
    explicit CombatantReference(QUuid combatantId, QObject *parent = nullptr);
    explicit CombatantReference(const CombatantReference &obj);  // copy constructor

    Combatant* getReference();
    const Combatant* getReference() const;
    QUuid getReferenceId();
    QUuid getReferenceId() const;

    // From Combatant - required
    virtual Combatant* clone() const;
    virtual int getSpeed() const;
    virtual int getStrength() const;
    virtual int getDexterity() const;
    virtual int getConstitution() const;
    virtual int getIntelligence() const;
    virtual int getWisdom() const;
    virtual int getCharisma() const;

    // From Combatant - additional
    virtual void inputXML(const QDomElement &element, bool isImport);
    virtual QString getName() const;
    virtual int getType() const;

signals:

public slots:

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

private:
    QUuid _referenceId;
    int _referenceIntId;
};

#endif // COMBATANTREFERENCE_H
