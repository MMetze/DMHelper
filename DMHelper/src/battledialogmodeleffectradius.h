#ifndef BATTLEDIALOGMODELEFFECTRADIUS_H
#define BATTLEDIALOGMODELEFFECTRADIUS_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectRadius : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectRadius();
    BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffectRadius(const BattleDialogModelEffect& other);
    virtual ~BattleDialogModelEffectRadius();

    virtual BattleDialogModelEffect* clone() const;

    virtual int getType() const;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const;
};

#endif // BATTLEDIALOGMODELEFFECTRADIUS_H
