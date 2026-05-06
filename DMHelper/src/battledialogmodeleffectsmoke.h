#ifndef BATTLEDIALOGMODELEFFECTSMOKE_H
#define BATTLEDIALOGMODELEFFECTSMOKE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectSmoke : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectSmoke(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectSmoke(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectSmoke() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QString getName() const override;

    virtual BattleDialogModelEffect* clone() const override;
    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;
    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

    qreal getDensity() const;
    void setDensity(qreal density);

    QColor getCenterColor() const;
    void setCenterColor(const QColor& color);

    QColor getEdgeColor() const;
    void setEdgeColor(const QColor& color);

    qreal getBillowFactor() const;
    void setBillowFactor(qreal billowFactor);

    qreal getWindDirection() const;
    void setWindDirection(qreal windDirection);

    qreal getWindStrength() const;
    void setWindStrength(qreal windStrength);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    qreal _density;
    QColor _centerColor;
    QColor _edgeColor;
    qreal _billowFactor;
    qreal _windDirection;
    qreal _windStrength;
};

#endif // BATTLEDIALOGMODELEFFECTSMOKE_H
