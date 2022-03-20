#ifndef BATTLEDIALOGMODELEFFECTLINE_H
#define BATTLEDIALOGMODELEFFECTLINE_H

#include "battledialogmodeleffectshape.h"

class BattleDialogModelEffectLine : public BattleDialogModelEffectShape
{
    Q_OBJECT

public:
    BattleDialogModelEffectLine(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectLine(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffectLine() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettings* getEffectEditor() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;
    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale) const override;

    virtual int getWidth() const override;
    virtual void setWidth(int width) override;

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    int _width;

};

#endif // BATTLEDIALOGMODELEFFECTLINE_H
