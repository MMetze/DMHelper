#ifndef COMBATANTFACTORY_H
#define COMBATANTFACTORY_H

#include <QObject>

class Combatant;
class QDomElement;
class QString;

class CombatantFactory : public QObject
{
    Q_OBJECT
public:
    explicit CombatantFactory(QObject *parent = 0);

    static Combatant* createCombatant(int combatantType, const QDomElement& element, QObject *parent = 0);

signals:

public slots:

};

#endif // COMBATANTFACTORY_H
