#ifndef BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
#define BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H

#include "battledialogmodeleffectobject.h"
#include "dmconstants.h"
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
    virtual BattleDialogEffectSettingsBase* getEffectEditor() const override;

    virtual bool hasEffectTransform() const override;

    virtual QGraphicsItem* createEffectShape(qreal gridScale) override;

    // Local Layer Interface
    virtual bool isPlayAudio() const;
    virtual DMHelper::TransparentType getEffectTransparencyType() const;
    virtual QColor getTransparentColor() const;
    virtual qreal getTransparentTolerance() const;
    virtual bool isColorize() const;
    virtual QColor getColorizeColor() const;
    virtual QPixmap getPixmap() const;

signals:
    void effectReady(BattleDialogModelEffect* effect);

public slots:
    void setPlayAudio(bool playAudio);
    void setEffectTransparencyType(DMHelper::TransparentType effectType);
    void setTransparentColor(const QColor& transparentColor);
    void setTransparentTolerance(qreal transparentTolerance);
    void setColorize(bool colorize);
    void setColorizeColor(const QColor& colorizeColor);

protected slots:
    void onScreenshotReady(const QImage& image);

protected:
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

    bool _playAudio;
    DMHelper::TransparentType _effectTransparencyType;
    QColor _transparentColor;
    qreal _transparentTolerance;
    bool _colorize;
    QColor _colorizeColor;

    VideoPlayerScreenshot* _screenshot;
    QPixmap _pixmap;
};

#endif // BATTLEDIALOGMODELEFFECTOBJECTVIDEO_H
