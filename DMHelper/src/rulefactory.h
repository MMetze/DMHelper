#ifndef RULEFACTORY_H
#define RULEFACTORY_H

#include <QObject>
#include <QHash>
#include <QDir>

class RuleInitiative;

class RuleFactory : public QObject
{
    Q_OBJECT
public:
    explicit RuleFactory(const QString& rulesetFile, QObject *parent = nullptr);

    static RuleFactory* Instance();
    static void Initialize(const QString& rulesetFile);
    static void Shutdown();

    static RuleInitiative* createRuleInitiative(const QString& ruleInitiativeType, QObject *parent = nullptr);
    static QString getRuleInitiativeDefault();
    static QStringList getRuleInitiativeNames();

    class RulesetTemplate;
    QList<QString> getRulesetNames() const;
    QList<RulesetTemplate> getRulesetTemplates() const;
    RulesetTemplate getRulesetTemplate(const QString& rulesetName) const;
    QDir getRulesetDir() const;

signals:

public slots:
    void readRuleset(const QString& rulesetFile);

private:
    static RuleFactory* _instance;

    QDir _rulesetDir;
    QHash<QString, RulesetTemplate> _rulesetTemplates;


public:

    class RulesetTemplate
    {
    public:
        RulesetTemplate() :
            _name(),
            _initiative(),
            _characterData(),
            _characterUI(),
            _monsterData(),
            _monsterUI()
        {}

        QString _name;
        QString _initiative;
        QString _characterData;
        QString _characterUI;
        QString _monsterData;
        QString _monsterUI;

    };

};

#endif // RULEFACTORY_H
