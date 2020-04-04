#ifndef ENCOUNTERTEXT_H
#define ENCOUNTERTEXT_H

#include "campaignobjectbase.h"

class QDomDocument;
class QDomElement;

class EncounterText : public CampaignObjectBase
{
    Q_OBJECT
public:

    explicit EncounterText(const QString& encounterName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    // Base functions to handle UI widgets
    //virtual void widgetActivated(QWidget* widget) override;
    //virtual void widgetDeactivated(QWidget* widget) override;

    virtual int getObjectType() const override;

    virtual QString getText() const;

public slots:
    virtual void setText(const QString& newText);

protected slots:
    //virtual void widgetChanged() override;
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

protected:
    void extractTextNode(const QDomElement &element, bool isImport);

    QString _text;
};

#endif // ENCOUNTERTEXT_H
