#ifndef PARTY_H
#define PARTY_H

#include "encountertext.h"
#include "scaledpixmap.h"

class Party : public EncounterText
{
    Q_OBJECT
public:
    explicit Party(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~Party() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual int getObjectType() const override;

    virtual QString getIcon(bool localOnly = false) const;
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize);

signals:
    void iconChanged();

public slots:
    virtual void setIcon(const QString &newIcon);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    QString _icon;
    ScaledPixmap _iconPixmap;
};

#endif // PARTY_H
