#ifndef COMBATANTFACTORY_H
#define COMBATANTFACTORY_H

#include "objectfactory.h"

class Combatant;
class QDomElement;
class QString;

class CombatantFactory : public ObjectFactory
{
    Q_OBJECT
public:
    explicit CombatantFactory(QObject *parent = nullptr);

    static Combatant* createCombatant(int combatantType, const QDomElement& element, bool isImport, QObject *parent = nullptr);

signals:

public slots:
    virtual CampaignObjectBase* createObject(int objectType, int subType, const QString& objectName, bool isImport) override;
    virtual CampaignObjectBase* createObject(const QDomElement& element, bool isImport) override;

};

#endif // COMBATANTFACTORY_H
