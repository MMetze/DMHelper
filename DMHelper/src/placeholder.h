#ifndef PLACEHOLDER_H
#define PLACEHOLDER_H

#include "campaignobjectbase.h"
#include <QList>

class Placeholder : public CampaignObjectBase
{
public:
    explicit Placeholder(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Placeholder() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    class PlaceHolderAttribute
    {
    public:
        PlaceHolderAttribute(const QString& name, const QString& value) : _name(name), _value(value) {}
        QString _name;
        QString _value;
    };

    QString _tagName;
    QList<PlaceHolderAttribute> _attributes;
};

#endif // PLACEHOLDER_H
