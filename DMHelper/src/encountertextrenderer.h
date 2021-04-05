#ifndef ENCOUNTERTEXTRENDERER_H
#define ENCOUNTERTEXTRENDERER_H

#include "campaignobjectrenderer.h"
#include "encountertext.h"
#include "videoplayer.h"
#include <QElapsedTimer>
#include <QImage>

class EncounterTextRenderer : public CampaignObjectRenderer
{
    Q_OBJECT
public:
    explicit EncounterTextRenderer(EncounterText& text, QImage backgroundImage, QObject *parent = nullptr);
    virtual ~EncounterTextRenderer() override;

public slots:
    virtual void startRendering() override;
    virtual void stopRendering() override;
    virtual void targetResized(const QSize& newSize) override;
    virtual void setRotation(int rotation) override;

    void rewind();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private slots:
    void startPublishTimer();
    void stopPublishTimer();

private:
    void prepareImages();
    void prepareTextImage();
    void drawTextImage(QPaintDevice* target);

    void createVideoPlayer();

    bool isVideo() const;
    bool isAnimated() const;
    QSize getRotatedTargetSize();
    QSize getRotatedSize(const QSize& size);

    EncounterText& _text;
    QSize _targetSize;
    int _rotation;
    bool _animationRunning;
    QPointF _textPos;
    QElapsedTimer _elapsed;
    int _timerId;

    QImage _backgroundImage;
    QImage _prescaledImage;
    QImage _textImage;

    VideoPlayer* _videoPlayer;
};

#endif // ENCOUNTERTEXTRENDERER_H
