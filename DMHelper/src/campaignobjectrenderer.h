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
    virtual void refreshRender() = 0;
    virtual void stopRendering() = 0;
    virtual void targetResized(const QSize& newSize) = 0;
    virtual void setRotation(int rotation) = 0;

    virtual void publishWindowMouseDown(const QPointF& position);
    virtual void publishWindowMouseMove(const QPointF& position);
    virtual void publishWindowMouseRelease(const QPointF& position);

signals:
    void showPublishWindow();
    void publishImage(QImage image);
    void animationStarted();
    void animateImage(QImage image);
    void animationStopped();
    void sendServerMessage(const QString& message);

};

#endif // CAMPAIGNOBJECTRENDERER_H
