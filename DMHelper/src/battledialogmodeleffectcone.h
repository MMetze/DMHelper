#ifndef BATTLEDIALOGMODELEFFECTCONE_H
#define BATTLEDIALOGMODELEFFECTCONE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectCone : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectCone(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectCone(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    //BattleDialogModelEffectCone(const BattleDialogModelEffectCone& other);
    virtual ~BattleDialogModelEffectCone() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const override;
};

#endif // BATTLEDIALOGMODELEFFECTCONE_H
