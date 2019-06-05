#ifndef BATTLEDIALOGMODELEFFECT_H
#define BATTLEDIALOGMODELEFFECT_H

#include "campaignobjectbase.h"
#include <QPointF>
#include <QColor>

class QAbstractGraphicsShapeItem;

const int BATTLE_DIALOG_MODEL_EFFECT_ID = 0;

class BattleDialogModelEffect : public CampaignObjectBase
{
    Q_OBJECT

public:
    enum
    {
        BattleDialogModelEffect_Base = 0,
        BattleDialogModelEffect_Radius,
        BattleDialogModelEffect_Cone,
        BattleDialogModelEffect_Cube,
        BattleDialogModelEffect_Line
    };

    BattleDialogModelEffect();
    BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    BattleDialogModelEffect(const BattleDialogModelEffect& other);
    virtual ~BattleDialogModelEffect();

    // From CampaignObjectBase
    virtual void outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport);
    virtual void inputXML(const QDomElement &element, bool isImport);

    virtual BattleDialogModelEffect* clone() const = 0;

    virtual int getType() const = 0;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const = 0;
    virtual void applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const;

    virtual int getSize() const;
    virtual void setSize(int size);

    virtual QPointF getPosition() const;
    virtual void setPosition(const QPointF& position);
    virtual void setPosition(qreal x, qreal y);

    virtual qreal getRotation() const;
    virtual void setRotation(qreal rotation);

    virtual QColor getColor() const;
    virtual void setColor(const QColor& color);

    virtual QString getTip() const;
    virtual void setTip(const QString& tip);

protected:

    void prepareItem(QAbstractGraphicsShapeItem& item) const;

    int _size;
    QPointF _position;
    qreal _rotation;
    QColor _color;
    QString _tip;

};


#endif // BATTLEDIALOGMODELEFFECT_H
