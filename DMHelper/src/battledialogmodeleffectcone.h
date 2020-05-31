#ifndef BATTLEDIALOGMODELEFFECTCONE_H
#define BATTLEDIALOGMODELEFFECTCONE_H

#include "battledialogmodeleffectshape.h"

class BattleDialogModelEffectCone : public BattleDialogModelEffectShape
{
    Q_OBJECT

public:
    BattleDialogModelEffectCone(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectCone(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    //BattleDialogModelEffectCone(const BattleDialogModelEffectCone& other);
    virtual ~BattleDialogModelEffectCone() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) const override;
};

#endif // BATTLEDIALOGMODELEFFECTCONE_H
