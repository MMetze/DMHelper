#ifndef CAMPAIGNOBJECTRENDERER_H
#define CAMPAIGNOBJECTRENDERER_H

#include <QObject>
#include <QImage>

class CampaignObjectRenderer : public QObject
{
    Q_OBJECT
public:
    explicit CampaignObjectRenderer(QObject *parent = nullptr);
    virtual ~CampaignObjectRenderer();

public slots:
    virtual void rendering(bool render);
    virtual void startRendering() = 0;
    virtual void stopRendering() = 0;
    virtual void targetResized(const QSize& newSize) = 0;
    virtual void setRotation(int rotation) = 0;

signals:
    void showPublishWindow();
    void publishImage(QImage image);
    void animationStarted();
    void animateImage(QImage image);
    void animationStopped();

};

#endif // CAMPAIGNOBJECTRENDERER_H
