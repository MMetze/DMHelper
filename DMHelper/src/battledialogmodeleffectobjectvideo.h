#ifndef BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
#define BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H

#include "battledialogmodeleffectobject.h"

class BattleDialogModelEffectObjectVideo : public BattleDialogModelEffectObject
{
    Q_OBJECT
public:
    BattleDialogModelEffectObjectVideo(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectObjectVideo(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip);
    virtual ~BattleDialogModelEffectObjectVideo() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual void setLayer(LayerTokens* tokensLayer) override;

    virtual int getEffectType() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

signals:
    void effectReady(BattleDialogModelEffect* effect);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

};

#endif // BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
