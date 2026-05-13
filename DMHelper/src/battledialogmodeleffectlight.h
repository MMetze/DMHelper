#ifndef BATTLEDIALOGMODELEFFECTLIGHT_H
#define BATTLEDIALOGMODELEFFECTLIGHT_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectLight : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    enum GradientType
    {
        GradientType_Linear = 0,
        GradientType_Quadratic
    };

    BattleDialogModelEffectLight(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectLight(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectLight() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QString getName() const override;

    virtual BattleDialogModelEffect* clone() const override;
    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;
    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

    int getGradientType() const;
    void setGradientType(int gradientType);

    bool getFlickerEnabled() const;
    void setFlickerEnabled(bool enabled);

    qreal getFlickerFrequency() const;
    void setFlickerFrequency(qreal frequency);

    qreal getDimAmplitude() const;
    void setDimAmplitude(qreal amplitude);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _gradientType;
    bool _flickerEnabled;
    qreal _flickerFrequency;
    qreal _dimAmplitude;
};

#endif // BATTLEDIALOGMODELEFFECTLIGHT_H
