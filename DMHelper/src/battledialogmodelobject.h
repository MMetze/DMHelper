#ifndef BATTLEDIALOGMODELOBJECT_H
#define BATTLEDIALOGMODELOBJECT_H

#include "campaignobjectbase.h"
#include <QPointF>

class QGraphicsItem;

class BattleDialogModelObject : public CampaignObjectBase
{
    Q_OBJECT
public:
    explicit BattleDialogModelObject(const QPointF& position = QPointF(), const QString& name = QString(), QObject *parent = nullptr);
    virtual ~BattleDialogModelObject() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual BattleDialogModelObject* getLinkedObject() const;
    QUuid getLinkedID() const;

    virtual QPointF getPosition() const;
    virtual void setPosition(const QPointF& position);
    virtual void setPosition(qreal x, qreal y);

    virtual void applyScale(QGraphicsItem& item, qreal gridScale);
    virtual qreal getScale();

public slots:
    virtual void setLinkedObject(BattleDialogModelObject* linkedObject);
    virtual void objectRemoved(BattleDialogModelObject* removedObject);
    virtual void setSelected(bool isSelected);
    virtual void parentMoved(const QPointF& delta);

signals:
    void linkChanged(BattleDialogModelObject* object, BattleDialogModelObject* previousLink);
    void objectMoved(BattleDialogModelObject* object);
    void objectMovedDelta(const QPointF& delta);

protected:
    // From CampaignObjectBase
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    BattleDialogModelObject* _linkedObject;
    QUuid _linkedId;
    QPointF _position;
};

#endif // BATTLEDIALOGMODELOBJECT_H
