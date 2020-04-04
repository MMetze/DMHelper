#ifndef PARTY_H
#define PARTY_H

#include "encountertext.h"

class Party : public EncounterText
{
    Q_OBJECT
public:
    explicit Party(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Party() override;

    virtual int getObjectType() const override;

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
};

#endif // PARTY_H
