#ifndef ENCOUNTERTEXT_H
#define ENCOUNTERTEXT_H

#include "encounter.h"

class QDomDocument;
class QDomElement;

class EncounterText : public Encounter
{
    Q_OBJECT
public:

    explicit EncounterText(const QString& encounterName, QObject *parent);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element);

    // Base functions to handle UI widgets
    virtual void widgetActivated(QWidget* widget);
    virtual void widgetDeactivated(QWidget* widget);

    virtual int getType() const;

    virtual QString getText() const;

public slots:
    virtual void setText(const QString& newText);

protected slots:
    virtual void widgetChanged();
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory);

protected:
    QString _text;

};

#endif // ENCOUNTERTEXT_H
