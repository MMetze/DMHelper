#ifndef RULEFACTORY_H
#define RULEFACTORY_H

#include <QObject>

class RuleInitiative;

class RuleFactory : public QObject
{
    Q_OBJECT
public:
    explicit RuleFactory(QObject *parent = nullptr);

    static RuleInitiative* createRuleInitiative(const QString& ruleInitiativeType, QObject *parent = nullptr);
    static QString getRuleInitiativeDefault();
    static QStringList getRuleInitiativeNames();

signals:
};

#endif // RULEFACTORY_H
