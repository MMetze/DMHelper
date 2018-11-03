#ifndef BATTLEDIALOGMODELEFFECTCONE_H
#define BATTLEDIALOGMODELEFFECTCONE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectCone : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectCone();
    BattleDialogModelEffectCone(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffectCone(const BattleDialogModelEffect& other);
    virtual ~BattleDialogModelEffectCone();

    virtual BattleDialogModelEffect* clone() const;

    virtual int getType() const;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const;
};

#endif // BATTLEDIALOGMODELEFFECTCONE_H
