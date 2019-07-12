#ifndef BATTLEDIALOGMODELEFFECTCUBE_H
#define BATTLEDIALOGMODELEFFECTCUBE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectCube : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectCube();
    explicit BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffectCube(const BattleDialogModelEffectCube& other);
    virtual ~BattleDialogModelEffectCube() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getType() const override;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const override;
};

#endif // BATTLEDIALOGMODELEFFECTCUBE_H
