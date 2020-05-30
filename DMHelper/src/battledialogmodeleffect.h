#ifndef BATTLEDIALOGMODELEFFECT_H
#define BATTLEDIALOGMODELEFFECT_H

#include "campaignobjectbase.h"
#include <QPointF>
#include <QColor>

class QAbstractGraphicsShapeItem;
class BattleDialogEffectSettings;
class QGraphicsItem;

const int BATTLE_DIALOG_MODEL_EFFECT_ID = 0;
const int BATTLE_DIALOG_MODEL_EFFECT_OBJECT = 1;

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

    BattleDialogModelEffect(const QString& name = QString(), QObject *parent = nullptr);
    BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffect() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual BattleDialogModelEffect* clone() const = 0;

    virtual int getEffectType() const = 0;
    virtual BattleDialogEffectSettings* getEffectEditor() const;

    virtual QAbstractGraphicsShapeItem* createEffectShape(qreal gridScale) const = 0;
    virtual void applyEffectValues(QAbstractGraphicsShapeItem& item, qreal gridScale) const;

    virtual bool getEffectActive() const;
    virtual void setEffectActive(bool active);

    virtual bool getEffectVisible() const;
    virtual void setEffectVisible(bool visible);

    virtual int getSize() const;
    virtual void setSize(int size);

    virtual int getWidth() const;
    virtual void setWidth(int width);

    virtual QPointF getPosition() const;
    virtual void setPosition(const QPointF& position);
    virtual void setPosition(qreal x, qreal y);

    virtual qreal getRotation() const;
    virtual void setRotation(qreal rotation);

    virtual QColor getColor() const;
    virtual void setColor(const QColor& color);

    virtual QString getTip() const;
    virtual void setTip(const QString& tip);

    void setEffectItemData(QGraphicsItem* item) const;
    static QUuid getEffectIdFromItem(QGraphicsItem* item);
    static BattleDialogModelEffect* getEffectFromItem(QGraphicsItem* item);
    static bool getEffectActiveFromItem(QGraphicsItem* item);
    static bool getEffectVisibleFromItem(QGraphicsItem* item);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    void prepareItem(QAbstractGraphicsShapeItem& item) const;
    void copyValues(const BattleDialogModelEffect &other);

    bool _active;
    bool _visible;
    int _size;
    QPointF _position;
    qreal _rotation;
    QColor _color;
    QString _tip;
};


#endif // BATTLEDIALOGMODELEFFECT_H
