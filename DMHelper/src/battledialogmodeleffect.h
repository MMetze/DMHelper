#ifndef BATTLEDIALOGMODELEFFECT_H
#define BATTLEDIALOGMODELEFFECT_H

#include "battledialogmodelobject.h"
#include <QPointF>
#include <QColor>

class BattleDialogEffectSettingsBase;
class LayerTokens;
class QAbstractGraphicsShapeItem;
class QGraphicsItem;

const int BATTLE_DIALOG_MODEL_EFFECT_ID = Qt::UserRole;
const int BATTLE_DIALOG_MODEL_EFFECT_OBJECT = Qt::UserRole + 1;
const int BATTLE_DIALOG_MODEL_EFFECT_ROLE = Qt::UserRole + 2;

class BattleDialogModelEffect : public BattleDialogModelObject
{
    Q_OBJECT

public:
    enum
    {
        BattleDialogModelEffect_Base = 0,
        BattleDialogModelEffect_Radius,
        BattleDialogModelEffect_Cone,
        BattleDialogModelEffect_Cube,
        BattleDialogModelEffect_Line,
        BattleDialogModelEffect_Object,
        BattleDialogModelEffect_ObjectVideo,

        BattleDialogModelEffect_Count
    };

    enum
    {
        BattleDialogModelEffectRole_None = 0,
        BattleDialogModelEffectRole_Area,
    };

    BattleDialogModelEffect(const QString& name = QString(), QObject *parent = nullptr);
    BattleDialogModelEffect(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip);
    virtual ~BattleDialogModelEffect() override;

    // From CampaignObjectBase
    virtual QDomElement outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory, bool isExport) override;
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;

    virtual BattleDialogModelEffect* clone() const = 0;

    virtual void setLayer(LayerTokens* tokensLayer);
    LayerTokens* getLayer() const;

    virtual int getEffectType() const = 0;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const;

    virtual void beginBatchChanges();
    virtual void endBatchChanges();

    virtual QGraphicsItem* createEffectShape(qreal gridScale) = 0;
    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale);
    virtual void applyScale(QGraphicsItem& item, qreal gridScale) override;
    virtual qreal getScale() override;

    virtual bool getEffectActive() const;
    virtual void setEffectActive(bool active);

    virtual bool getEffectVisible() const;
    virtual void setEffectVisible(bool visible);

    virtual int getSize() const;
    virtual void setSize(int size);

    virtual int getWidth() const;
    virtual void setWidth(int width);

    virtual bool hasEffectTransform() const;
    virtual void updateTransform(QGraphicsItem* graphicsItem) const;

    virtual QColor getColor() const;
    virtual void setColor(const QColor& color);

    virtual int getImageRotation() const;
    virtual void setImageRotation(int imageRotation);

    virtual QString getImageFile() const;
    virtual void setImageFile(const QString& imageFile);

    virtual QString getTip() const;
    virtual void setTip(const QString& tip);

    void setEffectItemData(QGraphicsItem* item) const;
    static QUuid getEffectIdFromItem(QGraphicsItem* item);
    static BattleDialogModelEffect* getEffectFromItem(QGraphicsItem* item);
    static BattleDialogModelEffect* getFinalEffect(BattleDialogModelEffect* effect);
    static bool getEffectActiveFromItem(QGraphicsItem* item);
    static bool getEffectVisibleFromItem(QGraphicsItem* item);

signals:
    void effectChanged(BattleDialogModelEffect* effect);

protected:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;

    virtual void prepareItem(QGraphicsItem& item) const;
    virtual void registerChange();

    LayerTokens* _tokensLayer;
    bool _active;
    bool _visible;
    int _size;
    qreal _rotation;
    QColor _color;
    QString _tip;

    bool _batchChanges;
    bool _changesMade;
};


#endif // BATTLEDIALOGMODELEFFECT_H
