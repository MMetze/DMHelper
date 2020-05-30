#ifndef BATTLEDIALOGMODELEFFECTRADIUS_H
#define BATTLEDIALOGMODELEFFECTRADIUS_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectRadius : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectRadius(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectRadius(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    //BattleDialogModelEffectRadius(const BattleDialogModelEffectRadius& other);
    virtual ~BattleDialogModelEffectRadius() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettings* getEffectEditor() const override;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const override;
};

#endif // BATTLEDIALOGMODELEFFECTRADIUS_H
