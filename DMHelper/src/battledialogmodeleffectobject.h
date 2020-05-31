#ifndef BATTLEDIALOGMODELEFFECTOBJECT_H
#define BATTLEDIALOGMODELEFFECTOBJECT_H

#include "battledialogmodeleffect.h"

class BattleDialogModelEffectObject : public BattleDialogModelEffect
{
    Q_OBJECT

public:
    BattleDialogModelEffectObject(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectObject(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip);
    virtual ~BattleDialogModelEffectObject() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual int getEffectType() const override;
    virtual BattleDialogEffectSettings* getEffectEditor() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) const override;
    virtual void applyEffectValues(QGraphicsItem& item, qreal gridScale) const override;

    virtual int getWidth() const override;
    virtual void setWidth(int width) override;

    virtual QString getImageFile() const override;
    virtual void setImageFile(const QString& imageFile) override;

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    virtual void prepareItem(QGraphicsItem& item) const override;

    int _width;
    QString _imageFile;

};

#endif // BATTLEDIALOGMODELEFFECTOBJECT_H
