#ifndef OVERLAYTIMER_H
#define OVERLAYTIMER_H

#include <QObject>
#include <QImage>
#include "overlay.h"

class PublishGLImage;
class QTimer;

class OverlayTimer : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayTimer(int seconds = 0, QObject *parent = nullptr);

    virtual int getOverlayType() const override;

    virtual int getTimerValue() const;

public slots:
    virtual void setTimerValue(int seconds);
    virtual void toggle(bool play);
    virtual void start();
    virtual void stop();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;

    virtual void createContentsGL() override;
    virtual void updateContentsGL() override;
    virtual void updateContentsScale(int w, int h) override;

    void createTimerImage();

    PublishGLImage* _timerPublishImage;
    QImage _timerImage;
    int _seconds;
    int _timerId;
};

#endif // OVERLAYTIMER_H
