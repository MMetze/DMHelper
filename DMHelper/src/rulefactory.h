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

    static const char* DEFAULT_RULESET_NAME;

    class RulesetTemplate;
    QList<QString> getRulesetNames() const;
    QList<RulesetTemplate> getRulesetTemplates() const;
    bool rulesetExists(const QString& rulesetName) const;
    RulesetTemplate getRulesetTemplate(const QString& rulesetName) const;
    QDir getRulesetDir() const;

    void setDefaultBestiary(const QString& bestiaryFile);
    QString getDefaultBestiary() const;

signals:

public slots:
    void readRuleset(const QString& rulesetFile);

private:
    static RuleFactory* _instance;

    QDir _rulesetDir;
    QString _defaultBestiary;
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
            _monsterUI(),
            _bestiary()
        {}

        QString _name;
        QString _initiative;
        QString _characterData;
        QString _characterUI;
        QString _monsterData;
        QString _monsterUI;
        QString _bestiary;

    };

};

#endif // RULEFACTORY_H
