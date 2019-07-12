#ifndef ADVENTUREITEM_H
#define ADVENTUREITEM_H

#include "campaignobjectbase.h"

class Adventure;

class AdventureItem : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit AdventureItem(QObject *parent);

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;

signals:

public slots:

public:
    Adventure* getAdventure() const;

};

#endif // ADVENTUREITEM_H
