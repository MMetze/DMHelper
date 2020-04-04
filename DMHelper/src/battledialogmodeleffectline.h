#ifndef BATTLEDIALOGMODELEFFECTLINE_H
#define BATTLEDIALOGMODELEFFECTLINE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectLine : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectLine(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectLine(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    //BattleDialogModelEffectLine(const BattleDialogModelEffectLine& other);
    virtual ~BattleDialogModelEffectLine() override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const override;
    virtual void applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const override;

protected:
};

#endif // BATTLEDIALOGMODELEFFECTLINE_H
