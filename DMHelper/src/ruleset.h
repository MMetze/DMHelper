#ifndef RULESET_H
#define RULESET_H

#include "campaignobjectbase.h"

class RuleInitiative;

class Ruleset : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit Ruleset(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Ruleset() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual int getObjectType() const override;
    virtual bool isTreeVisible() const override;

    // Local accessors
    RuleInitiative* getRuleInitiative();
    QString getRuleInitiativeType();
    QString getCharacterDataFile() const;
    QString getCharacterUIFile() const;
    QString getBestiaryFile() const;
    QString getMonsterDataFile() const;
    QString getMonsterUIFile() const;
    bool getCombatantDoneCheckbox() const;

    void setDefaultValues();

signals:
    void initiativeRuleChanged();
    void characterDataFileChanged(const QString& characterDataFile);
    void characterUIFileChanged(const QString& characterUIFile);
    void bestiaryFileChanged(const QString& bestiaryFile);
    void monsterDataFileChanged(const QString& monsterDataFile);
    void monsterUIFileChanged(const QString& monsterUIFile);

public slots:
    void setRuleInitiative(const QString& initiativeType);
    void setCharacterDataFile(const QString& characterDataFile);
    void setCharacterUIFile(const QString& characterUIFile);
    void setBestiaryFile(const QString& bestiaryFile);
    void setMonsterDataFile(const QString& monsterDataFile);
    void setMonsterUIFile(const QString& monsterUIFile);
    void setCombatantDoneCheckbox(bool checked);

protected slots:

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    RuleInitiative* _ruleInitiative;
    QString _characterDataFile;
    QString _characterUIFile;
    QString _bestiaryFile;
    QString _monsterDataFile;
    QString _monsterUIFile;
    bool _combatantDoneCheckbox;
};

#endif // RULESET_H
