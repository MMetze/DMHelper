#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "adventureitem.h"

class QDomDocument;
class QDomElement;

class Encounter : public AdventureItem
{
    Q_OBJECT
public:

    explicit Encounter(const QString& encounterName, QObject *parent);
    explicit Encounter(const Encounter& obj);

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);

    // Base functions to handle UI widgets
    virtual void widgetActivated(QWidget* widget) = 0;
    virtual void widgetDeactivated(QWidget* widget) = 0;

    virtual int getType() const;

    virtual QString getName() const;
    virtual void setName(const QString& newName);

    virtual bool hasData() const;

signals:
    void dirty();
    void changed();

public slots:

protected slots:
    virtual void widgetChanged() = 0;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) = 0;

protected:
    QString _name;
    QWidget* _widget;

};

#endif // ENCOUNTER_H
