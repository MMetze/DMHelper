#ifndef BATTLEDIALOGMODELEFFECTFIRE_H
#define BATTLEDIALOGMODELEFFECTFIRE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectFire : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectFire(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectFire(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectFire() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QString getName() const override;

    virtual BattleDialogModelEffect* clone() const override;
    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;
    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

    qreal getIntensity() const;
    void setIntensity(qreal intensity);

    QColor getDarkColor() const;
    void setDarkColor(const QColor& color);

    QColor getLightColor() const;
    void setLightColor(const QColor& color);

    qreal getFlickerSpeed() const;
    void setFlickerSpeed(qreal flickerSpeed);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    qreal _intensity;
    QColor _darkColor;
    QColor _lightColor;
    qreal _flickerSpeed;
};

#endif // BATTLEDIALOGMODELEFFECTFIRE_H
