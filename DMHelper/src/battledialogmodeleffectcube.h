#ifndef BATTLEDIALOGMODELEFFECTCUBE_H
#define BATTLEDIALOGMODELEFFECTCUBE_H

#include "battledialogmodeleffectshape.h"

class BattleDialogModelEffectCube : public BattleDialogModelEffectShape
{
    Q_OBJECT

public:
    BattleDialogModelEffectCube(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectCube() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;
};

#endif // BATTLEDIALOGMODELEFFECTCUBE_H
