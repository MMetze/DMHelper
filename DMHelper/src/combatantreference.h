#ifndef COMBATANTREFERENCE_H
#define COMBATANTREFERENCE_H

#include "campaignobjectbase.h"
#include "combatant.h"
#include <QUuid>

class CombatantReference : public Combatant
{
    Q_OBJECT
public:
    explicit CombatantReference(const QString& name = QString(), QObject *parent = nullptr);
    explicit CombatantReference(const Combatant &combatant, QObject *parent = nullptr);
    explicit CombatantReference(QUuid combatantId, QObject *parent = nullptr);
    //explicit CombatantReference(const CombatantReference &obj);  // copy constructor

    Combatant* getReference();
    const Combatant* getReference() const;
    QUuid getReferenceId();
    QUuid getReferenceId() const;

    // From Combatant - required
    virtual Combatant* clone() const override;
    virtual int getSpeed() const override;
    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    // From Combatant - additional
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual QString getName() const override;
    virtual int getCombatantType() const override;

signals:
    void referenceChanged();

public slots:
    void setReference(const Combatant& combatant);
    void setReference(const QUuid& combatantId);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

private:
    QUuid _referenceId;
    int _referenceIntId;
};

#endif // COMBATANTREFERENCE_H
