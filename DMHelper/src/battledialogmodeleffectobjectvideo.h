#ifndef BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
#define BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H

#include "battledialogmodeleffectobject.h"
#include <QPixmap>

class VideoPlayerScreenshot;

class BattleDialogModelEffectObjectVideo : public BattleDialogModelEffectObject
{
    Q_OBJECT
public:
    BattleDialogModelEffectObjectVideo(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectObjectVideo(int size, int width, const QPointF& position, qreal rotation, const QString& videoFile, const QString& tip);
    virtual ~BattleDialogModelEffectObjectVideo() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;

    virtual BattleDialogModelEffect* clone() const override;

    virtual void setLayer(LayerTokens* tokensLayer) override;

    virtual int getEffectType() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

signals:
    void effectReady(BattleDialogModelEffect* effect);

protected slots:
    void onScreenshotReady(const QImage& image);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    VideoPlayerScreenshot* _screenshot;
    QPixmap _pixmap;
};

#endif // BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
