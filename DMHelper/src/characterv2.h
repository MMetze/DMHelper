#ifndef CHARACTERV2_H
#define CHARACTERV2_H

#include "combatant.h"
#include "templateobject.h"
#include <QHash>
#include <QVariant>

class DMHAttribute;

class Characterv2 : public Combatant, public TemplateObject
{
    Q_OBJECT
public:

    explicit Characterv2(const QString& name = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QIcon getDefaultIcon() override;

    // For support of GlobalSearch_Interface
    virtual bool matchSearch(const QString& searchString, QString& result) const override;

    // From Combatant
    virtual void beginBatchChanges() override;
    virtual void endBatchChanges() override;

    virtual Combatant* clone() const override;

    virtual int getCombatantType() const override;

    // Local Character Values
    virtual int getDndBeyondID() const;
    virtual void setDndBeyondID(int id);

    virtual bool isInParty() const;

    virtual int getSpeed() const override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

signals:
    void iconChanged(CampaignObjectBase* character);

public slots:
    virtual void setIcon(const QString &newIcon) override;

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    // From TemplateObject
    virtual QHash<QString, QVariant>* valueHash() override;
    virtual const QHash<QString, QVariant>* valueHash() const override;
    virtual void declareDirty() override;
    virtual void handleOldXMLs(const QDomElement& element) override;
    virtual bool isAttributeSpecial(const QString& attribute) const override;
    virtual QVariant getAttributeSpecial(const QString& attribute) const override;
    virtual QString getAttributeSpecialAsString(const QString& attribute) const override;
    virtual void setAttributeSpecial(const QString& key, const QString& value) override;

private:

    int _dndBeyondID;
    bool _iconChanged;

    QHash<QString, QVariant> _allValues;

};

#endif // CHARACTERV2_H
