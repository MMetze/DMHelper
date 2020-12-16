#ifndef ENCOUNTERTEXT_H
#define ENCOUNTERTEXT_H

#include "campaignobjectbase.h"
#include <QDomElement>

class QDomDocument;

class EncounterText : public CampaignObjectBase
{
    Q_OBJECT
public:

    explicit EncounterText(const QString& encounterName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;

    virtual QString getText() const;

public slots:
    virtual void setText(const QString& newText);

signals:
    void textChanged(const QString& text);

protected slots:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

protected:
    void extractTextNode(const QDomElement &element, bool isImport);

    QString _text;
};

#endif // ENCOUNTERTEXT_H
