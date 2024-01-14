#ifndef PARTY_H
#define PARTY_H

#include "encountertext.h"
#include "scaledpixmap.h"

class Character;

class Party : public EncounterText
{
    Q_OBJECT
public:
    explicit Party(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Party() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;
    virtual QIcon getDefaultIcon() override;

    virtual QString getPartyIcon(bool localOnly = false) const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);

    QList<Character*> getActiveCharacters();

signals:
    void iconChanged(CampaignObjectBase* party);

public slots:
    virtual void setIcon(const QString &newIcon);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QString _icon;
    ScaledPixmap _iconPixmap;
};

#endif // PARTY_H
