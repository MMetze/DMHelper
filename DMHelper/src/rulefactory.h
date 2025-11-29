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
    explicit RuleFactory(const QString& defaultRulesetFile, const QString& userRulesetFile, QObject *parent = nullptr);

    static RuleFactory* Instance();
    static void Initialize(const QString& defaultRulesetFile, const QString& userRulesetFile);
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

    void setDefaultBestiary(const QString& bestiaryFile);
    QString getDefaultBestiary() const;

signals:

public slots:
    void readRuleset(const QString& rulesetFile);

private:
    static RuleFactory* _instance;

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
            _bestiary(),
            _rulesetDir(),
            _combatantDone(false),
            _hitPointsCountDown(true),
            _movement()
        {}

        RulesetTemplate(const QString& name, const QString& initiative, const QString& characterData,
                        const QString& characterUI, const QString& monsterData, const QString& monsterUI,
                        const QString& bestiary, QDir _rulesetDir, const QString& movement,
                        bool combatantDone = false, bool hitPointsCountDown = true) :
            _name(name),
            _initiative(initiative),
            _characterData(characterData),
            _characterUI(characterUI),
            _monsterData(monsterData),
            _monsterUI(monsterUI),
            _bestiary(bestiary),
            _rulesetDir(_rulesetDir),
            _combatantDone(combatantDone),
            _hitPointsCountDown(hitPointsCountDown),
            _movement(movement)
        {}

        QString _name;
        QString _initiative;
        QString _characterData;
        QString _characterUI;
        QString _monsterData;
        QString _monsterUI;
        QString _bestiary;
        QDir _rulesetDir;
        bool _combatantDone;
        bool _hitPointsCountDown;
        QString _movement;

    };

};

#endif // RULEFACTORY_H
