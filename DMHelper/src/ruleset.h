#ifndef RULESET_H
#define RULESET_H

#include <QObject>

class Ruleset : public QObject
{
    Q_OBJECT
public:
    explicit Ruleset(QObject *parent = nullptr);

signals:
};

#endif // RULESET_H
