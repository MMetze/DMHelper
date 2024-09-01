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
    bool getCombatantDoneCheckbox() const;

signals:
    void initiativeRuleChanged();
    void characterDataFileChanged(const QString& characterDataFile);
    void characterUIFileChanged(const QString& characterUIFile);

public slots:
    void setRuleInitiative(const QString& initiativeType);
    void setCharacterDataFile(const QString& characterDataFile);
    void setCharacterUIFile(const QString& characterUIFile);
    void setCombatantDoneCheckbox(bool checked);

protected slots:

protected:
    // From CampaignObjectBase
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    RuleInitiative* _ruleInitiative;
    QString _characterDataFile;
    QString _characterUIFile;
    bool _combatantDoneCheckbox;
};

#endif // RULESET_H
