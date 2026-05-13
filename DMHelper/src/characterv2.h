#ifndef CHARACTERV2_H
#define CHARACTERV2_H

#include "combatant.h"
#include "templateobject.h"
#include "scaledpixmap.h"
#include <QHash>
#include <QVariant>
#include <QStringList>

class DMHAttribute;
class MonsterClassv2;

class Characterv2 : public Combatant, public TemplateObject
{
    Q_OBJECT
public:

    explicit Characterv2(const QString& name = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QIcon getDefaultIcon() override;

    // For support of GlobalSearch_Interface
    virtual bool matchSearch(const QString& searchString, QString& result) const override;

    // From Combatant
    virtual void beginBatchChanges() override;
    virtual void endBatchChanges() override;

    virtual Combatant* clone() const override;

    virtual int getCombatantType() const override;

    // Local Character Values
    virtual int getDndBeyondID() const;
    virtual void setDndBeyondID(int id);

    virtual bool isInParty() const;

    virtual int getSpeed() const override;

    virtual int getStrength() const override;
    virtual int getDexterity() const override;
    virtual int getConstitution() const override;
    virtual int getIntelligence() const override;
    virtual int getWisdom() const override;
    virtual int getCharisma() const override;

    virtual void copyMonsterValues(MonsterClassv2& monster);

    // Multi-icon support
    int getIconCount() const;
    QStringList getIconList() const;
    QString getIcon(int index = 0) const;
    QPixmap getIconPixmap(DMHelper::PixmapSize iconSize, int index);
    virtual QPixmap getIconPixmap(DMHelper::PixmapSize iconSize) override;
    virtual QString getIconFile() const override;

signals:
    void iconChanged(CampaignObjectBase* character);

public slots:
    virtual void setIcon(const QString &newIcon) override;
    void addIcon(const QString& iconFile);
    void setIcon(int index, const QString& iconFile);
    void removeIcon(int index);
    void removeIcon(const QString& iconFile);
    void clearIcon();
    void refreshIconPixmaps();

protected:
    // From Combatant
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    // From TemplateObject
    virtual QHash<QString, QVariant>* valueHash() override;
    virtual const QHash<QString, QVariant>* valueHash() const override;
    virtual void declareDirty() override;
    virtual void handleOldXMLs(const QDomElement& element) override;
    virtual bool isAttributeSpecial(const QString& attribute) const override;
    virtual QVariant getAttributeSpecial(const QString& attribute) const override;
    virtual QString getAttributeSpecialAsString(const QString& attribute) const override;
    virtual void setAttributeSpecial(const QString& key, const QString& value) override;

    void readIcons(const QDomElement& element, bool isImport);
    void writeIcons(QDomDocument &doc, QDomElement& element, QDir& targetDirectory, bool isExport) const;

private:

    int _dndBeyondID;
    bool _iconChanged;

    QHash<QString, QVariant> _allValues;

    QStringList _icons;
    QList<ScaledPixmap> _scaledPixmaps;

};

#endif // CHARACTERV2_H
