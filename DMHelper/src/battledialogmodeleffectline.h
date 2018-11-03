#ifndef BATTLEDIALOGMODELEFFECTLINE_H
#define BATTLEDIALOGMODELEFFECTLINE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectLine : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectLine();
    BattleDialogModelEffectLine(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffectLine(const BattleDialogModelEffect& other);
    virtual ~BattleDialogModelEffectLine();

    virtual BattleDialogModelEffect* clone() const;

    virtual int getType() const;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const;
    virtual void applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const;
};

#endif // BATTLEDIALOGMODELEFFECTLINE_H
