#ifndef TEMPLATEOBJECT_H
#define TEMPLATEOBJECT_H

#include "dice.h"
#include "combatant.h"
#include <QObject>
#include <QHash>
#include <QVariant>

class TemplateFactory;
class DMHAttribute;

class TemplateObject
{
public:
    TemplateObject(TemplateFactory* factory);
    virtual ~TemplateObject();

    TemplateFactory* getFactory() const;
    void setFactory(TemplateFactory* factory);

    // For support of GlobalSearch_Interface
    bool matchSearchString(const QString& searchString, QString& result) const;

    virtual bool hasValue(const QString& key) const;
    virtual QString getValueAsString(const QString& key) const;
    virtual QString getStringValue(const QString& key) const;
    virtual int getIntValue(const QString& key) const;
    virtual bool getBoolValue(const QString& key) const;
    virtual Dice getDiceValue(const QString& key) const;
    virtual ResourcePair getResourceValue(const QString& key) const;
    virtual QList<QVariant> getListValue(const QString& key) const;

    virtual void setValue(const QString& key, const QVariant& value);
    virtual void setValue(const QString& key, const QString& value);
    virtual void setStringValue(const QString& key, const QString& value);
    virtual void setIntValue(const QString& key, int value);
    virtual void setBoolValue(const QString& key, bool value);
    virtual void setDiceValue(const QString& key, const Dice& value);
    virtual void setResourceValue(const QString& key, const ResourcePair& value);
    virtual void setListValue(const QString& key, int index, const QString& listEntryKey, const QVariant& listEntryValue);
    virtual void setListValue(const QString& key, int index, const QString& listEntryKey, const QString& listEntryValue);
    virtual QHash<QString, QVariant> createListEntry(const QString& key, int index);
    virtual void insertListEntry(const QString& key, int index, QHash<QString, QVariant> listEntryValues);
    virtual void prependListEntry(const QString& key, QHash<QString, QVariant> listEntryValues);
    virtual void appendListEntry(const QString& key, QHash<QString, QVariant> listEntryValues);
    virtual void removeListEntry(const QString& key, int index);

protected:
    virtual QHash<QString, QVariant>* valueHash() = 0;
    virtual const QHash<QString, QVariant>* valueHash() const = 0;
    virtual void declareDirty() = 0;

    virtual void readXMLValues(const QDomElement& element, bool isImport);
    virtual void writeXMLValues(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;

    virtual void handleOldXMLs(const QDomElement& element);
    virtual bool isAttributeSpecial(const QString& attribute) const;
    virtual QVariant getAttributeSpecial(const QString& attribute) const;
    virtual QString getAttributeSpecialAsString(const QString& attribute) const;
    virtual void setAttributeSpecial(const QString& key, const QString& value);

    virtual QVariant readAttributeValue(const DMHAttribute& attribute, const QDomElement& element, const QString& name);
    virtual void writeAttributeValue(const DMHAttribute& attribute, QDomElement& element, const QString& key, const QVariant& value) const;
    virtual void writeElementValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value) const;
    virtual void writeElementListValue(QDomDocument &doc, QDomElement& element, const QString& key, const QVariant& value) const;

    TemplateFactory* _factory;
};

#endif // TEMPLATEOBJECT_H
