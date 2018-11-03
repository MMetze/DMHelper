#ifndef ENCOUNTERFACTORY_H
#define ENCOUNTERFACTORY_H

#include <QObject>

class Encounter;
class QDomElement;
class QString;

class EncounterFactory : public QObject
{
    Q_OBJECT
public:
    explicit EncounterFactory(QObject *parent = 0);

    static Encounter* createEncounter(int encounterType, const QString& encounterName, QObject *parent);
    static Encounter* createEncounter(int encounterType, const QDomElement& element, QObject *parent);

signals:

public slots:

private:

};

#endif // ENCOUNTERFACTORY_H
