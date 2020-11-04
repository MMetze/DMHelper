#ifndef BATTLEDIALOGMODELEFFECTSHAPE_H
#define BATTLEDIALOGMODELEFFECTSHAPE_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectShape : public BattleDialogModelEffect
{
    Q_OBJECT
public:
    BattleDialogModelEffectShape(const QString& name = QString(), QObject *parent = nullptr);
    BattleDialogModelEffectShape(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectShape() override;

    // From CampaignObjectBase
    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale) const override;

protected:

};

#endif // BATTLEDIALOGMODELEFFECTSHAPE_H
