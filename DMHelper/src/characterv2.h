#ifndef CHARACTERV2_H
#define CHARACTERV2_H

#include "combatant.h"
#include <QHash>

class MonsterAction;
class DMHAttribute;

class Characterv2 : public Combatant
{
    Q_OBJECT
public:

    explicit Characterv2(const QString& name = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QIcon getDefaultIcon() override;

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

    QString getValueAsString(const QString& key) const;
    QString getStringValue(const QString& key) const;
    int getIntValue(const QString& key) const;
    bool getBoolValue(const QString& key) const;
    Dice getDiceValue(const QString& key) const;
    ResourcePair getResourceValue(const QString& key) const;
    QList<QVariant> getListValue(const QString& key) const;

signals:
    void iconChanged(CampaignObjectBase* character);

public slots:
    virtual void setIcon(const QString &newIcon) override;
    void setValue(const QString& key, const QVariant& value);
    void setValue(const QString& key, const QString& value);
    void setStringValue(const QString& key, const QString& value);
    void setIntValue(const QString& key, int value);
    void setBoolValue(const QString& key, bool value);
    void setDiceValue(const QString& key, const Dice& value);
    void setResourceValue(const QString& key, const ResourcePair& value);
    void setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue);

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

private:
    //void readActionList(const QDomElement& element, const QString& actionName, QList<MonsterAction>& actionList, bool isImport);
    //void writeActionList(QDomDocument &doc, QDomElement& element, const QString& actionName, const QList<MonsterAction>& actionList, bool isExport) const;
    void readXMLValues(const QDomElement& element, bool isImport);
    void handleOldXMLs(const QDomElement& element);
    bool isAttributeSpecial(const QString& attribute) const;
    QVariant getAttributeSpecial(const QString& attribute) const;
    QString getAttributeSpecialAsString(const QString& attribute) const;
    void setAttributeSpecial(const QString& key, const QString& value);
    QVariant readAttributeValue(const DMHAttribute& attribute, const QDomElement& element, const QString& name);
    void writeAttributeValue(const DMHAttribute& attribute, QDomElement& element, const QString& key, const QVariant& value);
    void writeElementValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value);
    void writeElementListValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value);

    int _dndBeyondID;
    bool _iconChanged;

    QHash<QString, QVariant> _allValues;
    /*
    QHash<QString, QString> _stringValues;
    QHash<QString, int> _intValues;
    QHash<QString, bool> _boolValues;
    QHash<QString, Dice> _diceValues;
    QHash<QString, ResourcePair> _resourceValues;
    QHash<QString, QString> _htmlValues;
    QHash<QString, QList<DMHAttribute>> _elementLists;
*/
};

#endif // CHARACTERV2_H
