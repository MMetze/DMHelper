#ifndef BATTLEDIALOGMODELEFFECTOBJECT_H
#define BATTLEDIALOGMODELEFFECTOBJECT_H

#include "battledialogmodeleffect.h"
#include <QSizeF>

class BattleDialogModelEffectObject : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectObject(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip);
    virtual ~BattleDialogModelEffectObject() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual QString getName() const override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;
    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale) override;
    virtual void applyScale(QGraphicsItem& item, qreal gridScale) override;
    virtual qreal getScale() override;

    virtual int getWidth() const override;
    virtual void setWidth(int width) override;

    virtual bool hasEffectTransform() const override;
    virtual void updateTransform(QGraphicsItem* graphicsItem) const override;

    virtual int getImageRotation() const override;
    virtual void setImageRotation(int imageRotation) override;

    virtual QString getImageFile() const override;
    virtual void setImageFile(const QString& imageFile) override;

    qreal getImageScaleFactor() const;

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    virtual void prepareItem(QGraphicsItem& item) const override;
    virtual QGraphicsItem* createPixmapShape(qreal gridScale, const QPixmap& pixmap);

    int _width;
    int _imageRotation;
    qreal _gridScale;
    QSizeF _imageScaleFactors;
    QString _imageFile;

};

#endif // BATTLEDIALOGMODELEFFECTOBJECT_H
