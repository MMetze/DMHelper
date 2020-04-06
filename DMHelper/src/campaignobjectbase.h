#ifndef CAMPAIGNOBJECTBASE_H
#define CAMPAIGNOBJECTBASE_H

#include "dmhobjectbase.h"
#include <QList>

class Campaign;

class CampaignObjectBase : public DMHObjectBase
{
    Q_OBJECT
public:

    // Allow full access to the Campaign class
    explicit CampaignObjectBase(const QString& name = QString(), QObject *parent = nullptr);
    virtual ~CampaignObjectBase() override;

    explicit CampaignObjectBase(const CampaignObjectBase& other) = delete;
    CampaignObjectBase& operator=(const CampaignObjectBase& other) = delete;

    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void postProcessXML(const QDomElement &element, bool isImport) override;
    //virtual void resolveReferences();

    // Base functions to handle UI widgets
    //virtual void widgetActivated(QWidget* widget) = 0;
    //virtual void widgetDeactivated(QWidget* widget) = 0;

    virtual int getObjectType() const;
    virtual bool getExpanded() const;
    virtual QString getName() const;

    const QList<CampaignObjectBase*> getChildObjects() const;
    QList<CampaignObjectBase*> getChildObjects();
    CampaignObjectBase* getChildById(QUuid id);

    const CampaignObjectBase* getParentByType(int parentType) const;
    CampaignObjectBase* getParentByType(int parentType);

    const CampaignObjectBase* getParentById(const QUuid& id) const;
    CampaignObjectBase* getParentById(const QUuid& id);

    QUuid addObject(CampaignObjectBase* object);
    CampaignObjectBase* removeObject(QUuid id);

    CampaignObjectBase* getObjectById(QUuid id);
    const CampaignObjectBase* getObjectById(QUuid id) const;

signals:
    void changed();
    void dirty();
    void expandedChanged(bool expanded);
    void nameChanged(const QString& name);

public slots:
    void setExpanded(bool expanded);
    void setName(const QString& name);

protected slots:
    virtual void handleInternalChange();
    virtual void handleInternalDirty();

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) = 0;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element);

    QUuid parseIdString(QString idString, int* intId = nullptr, bool isLocal = false);
    QUuid findUuid(int intId) const;
    QUuid findChildUuid(int intId) const;

    bool _expanded;
};

#endif // CAMPAIGNOBJECTBASE_H
