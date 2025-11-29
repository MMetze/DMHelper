#ifndef RULESET_H
#define RULESET_H

#include "campaignobjectbase.h"
#include "rulefactory.h"
#include "dmconstants.h"

class RuleInitiative;

class Ruleset : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Ruleset(const QString& name = QString(), QObject *parent = nullptr);
    explicit Ruleset(const RuleFactory::RulesetTemplate& rulesetTemplate, QObject *parent = nullptr);
    virtual ~Ruleset() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual int getObjectType() const override;
    virtual bool isTreeVisible() const override;

    // Local interface
    void setValues(const RuleFactory::RulesetTemplate& rulesetTemplate);
    void startBatchProcessing();
    void endBatchProcessing();

    // Local accessors
    bool isInitialized() const;
    RuleInitiative* getRuleInitiative();
    QString getRuleInitiativeType();
    QString getCharacterDataFile() const;
    QString getCharacterUIFile() const;
    QString getBestiaryFile() const;
    QString getMonsterDataFile() const;
    QString getMonsterUIFile() const;
    bool getCombatantDoneCheckbox() const;
    bool getHitPointsCoundDown() const;
    QString getMovementString() const;
    DMHelper::MovementType getMovementType() const;
    QList<int> getMovementRanges() const;

    static DMHelper::MovementType movementTypeFromString(const QString& movementStr, QList<int>* movementRanges = nullptr);
    static QString movementStringFromType(DMHelper::MovementType movementType, const QList<int>* movementRanges = nullptr);

signals:
    void rulesetChanged();

public slots:
    void setRuleInitiative(const QString& initiativeType);
    void setCharacterDataFile(const QString& characterDataFile);
    void setCharacterUIFile(const QString& characterUIFile);
    void setBestiaryFile(const QString& bestiaryFile);
    void setMonsterDataFile(const QString& monsterDataFile);
    void setMonsterUIFile(const QString& monsterUIFile);
    void setCombatantDoneCheckbox(bool checked);
    void setHitPointsCountDown(bool countDown);
    void setMovementString(const QString& movement);
    void setMovementType(DMHelper::MovementType type);
    void setMovementRanges(QList<int> ranges);

protected slots:

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    bool areSameFile(const QString& file1, const QString& file2) const;

    void registerChange();

    RuleInitiative* _ruleInitiative;
    QString _characterDataFile;
    QString _characterUIFile;
    QString _bestiaryFile;
    QString _monsterDataFile;
    QString _monsterUIFile;
    bool _combatantDoneCheckbox;
    bool _hitPointsCountDown;
    DMHelper::MovementType _movementType;
    QList<int> _movementRanges;

    bool _batchProcessing;
    bool _changed;
};

#endif // RULESET_H
