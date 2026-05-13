#ifndef BATTLEDIALOGMODELEFFECTSPARKS_H
#define BATTLEDIALOGMODELEFFECTSPARKS_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectSparks : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectSparks(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectSparks(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectSparks() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QString getName() const override;

    virtual BattleDialogModelEffect* clone() const override;
    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;
    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

    int getParticleCount() const;
    void setParticleCount(int count);

    qreal getGlowRadius() const;
    void setGlowRadius(qreal radius);

    qreal getGlowOpacity() const;
    void setGlowOpacity(qreal opacity);

    bool getArcFalloff() const;
    void setArcFalloff(bool arcFalloff);

    bool getFadeDistance() const;
    void setFadeDistance(bool fadeDistance);

    qreal getSparkSpeed() const;
    void setSparkSpeed(qreal speed);

    qreal getWindDirection() const;
    void setWindDirection(qreal windDirection);

    qreal getWindStrength() const;
    void setWindStrength(qreal windStrength);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _particleCount;
    qreal _glowRadius;
    qreal _glowOpacity;
    bool _arcFalloff;
    bool _fadeDistance;
    qreal _sparkSpeed;
    qreal _windDirection;
    qreal _windStrength;
};

#endif // BATTLEDIALOGMODELEFFECTSPARKS_H
