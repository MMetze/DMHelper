#ifndef CAMPAIGNOBJECTBASE_H
#define CAMPAIGNOBJECTBASE_H

#include "dmhobjectbase.h"
#include <QList>
#include <QDataStream>

class CampaignObjectBase : public DMHObjectBase
{
    Q_OBJECT
public:

    explicit CampaignObjectBase(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~CampaignObjectBase() override;

    explicit CampaignObjectBase(const CampaignObjectBase& other) = delete;
    CampaignObjectBase& operator=(const CampaignObjectBase& other) = delete;

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport);
    virtual void copyValues(const CampaignObjectBase* other);

    virtual int getObjectType() const;
    virtual bool getExpanded() const;
    virtual QString getName() const;
    virtual int getRow() const;
    virtual bool isTreeVisible() const;

    virtual QIcon getIcon();
    virtual QIcon getDefaultIcon();
    virtual QString getIconFile() const;

    const QList<CampaignObjectBase*> getChildObjects() const;
    QList<CampaignObjectBase*> getChildObjects();
    QList<CampaignObjectBase*> getChildObjectsByType(int childType);
    CampaignObjectBase* getChildById(QUuid id);
    CampaignObjectBase* searchChildrenById(QUuid id);
    CampaignObjectBase* searchDirectChildrenByName(const QString& childName);

    virtual const CampaignObjectBase* getParentByType(int parentType) const;
    virtual CampaignObjectBase* getParentByType(int parentType);

    virtual const CampaignObjectBase* getParentById(const QUuid& id) const;
    virtual CampaignObjectBase* getParentById(const QUuid& id);

    QUuid addObject(CampaignObjectBase* object);
    CampaignObjectBase* removeObject(CampaignObjectBase* object);
    CampaignObjectBase* removeObject(QUuid id);

    CampaignObjectBase* getObjectById(QUuid id);
    const CampaignObjectBase* getObjectById(QUuid id) const;

signals:
    void changed();
    void dirty();
    void expandedChanged(bool expanded);
    void nameChanged(CampaignObjectBase* object, const QString& name);
    void iconFileChanged(CampaignObjectBase* object);
    void campaignObjectDestroyed(const QUuid& id);

public slots:
    void setExpanded(bool expanded);
    void setName(const QString& name);
    void setRow(int row);
    void setIconFile(const QString& iconFile);

protected slots:
    virtual void handleInternalChange();
    virtual void handleInternalDirty();

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) = 0;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element);
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport);

    QUuid parseIdString(QString idString, int* intId = nullptr, bool isLocal = false);
    QUuid findUuid(int intId) const;
    QUuid findChildUuid(int intId) const;

    bool _expanded;
    int _row;
    QString _iconFile;
#ifdef QT_DEBUG
    QString _DEBUG_NAME;
#endif
};

Q_DECLARE_METATYPE(CampaignObjectBase*)
Q_DECLARE_METATYPE(const CampaignObjectBase*)

#endif // CAMPAIGNOBJECTBASE_H
