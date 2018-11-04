#ifndef BATTLEDIALOGMODELEFFECTCUBE_H
#define BATTLEDIALOGMODELEFFECTCUBE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectCube : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectCube();
    BattleDialogModelEffectCube(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffectCube(const BattleDialogModelEffect& other);
    virtual ~BattleDialogModelEffectCube();

    virtual BattleDialogModelEffect* clone() const;

    virtual int getType() const;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const;
};

#endif // BATTLEDIALOGMODELEFFECTCUBE_H
