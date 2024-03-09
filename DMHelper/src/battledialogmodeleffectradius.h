#ifndef BATTLEDIALOGMODELEFFECTRADIUS_H
#define BATTLEDIALOGMODELEFFECTRADIUS_H

#include "battledialogmodeleffectshape.h"

class BattleDialogModelEffectRadius : public BattleDialogModelEffectShape
{
    Q_OBJECT

public:
    BattleDialogModelEffectRadius(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    //BattleDialogModelEffectRadius(const BattleDialogModelEffectRadius& other);
    virtual ~BattleDialogModelEffectRadius() override;

    virtual QString getName() const override;
    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;
};

#endif // BATTLEDIALOGMODELEFFECTRADIUS_H
