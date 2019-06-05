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
    explicit CombatantFactory(QObject *parent = nullptr);

    static Combatant* createCombatant(int combatantType, const QDomElement& element, QObject *parent = nullptr);

signals:

public slots:

};

#endif // COMBATANTFACTORY_H
