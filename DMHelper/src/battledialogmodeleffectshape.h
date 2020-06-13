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
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale) const override;

    virtual QColor getColor() const override;
    virtual void setColor(const QColor& color) override;

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    void copyValues(const BattleDialogModelEffectShape &other);

    QColor _color;
};

#endif // BATTLEDIALOGMODELEFFECTSHAPE_H
