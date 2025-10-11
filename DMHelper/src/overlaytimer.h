#ifndef OVERLAYTIMER_H
#define OVERLAYTIMER_H

#include <QObject>
#include "overlay.h"

class PublishGLImage;
class QTimer;

class OverlayTimer : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayTimer(int seconds = 0, QObject *parent = nullptr);

    virtual int getOverlayType() const override;

    void setTimerValue(int seconds);
    int getTimerValue() const;

    void start();
    void stop();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;

    virtual void createContentsGL() override;
    virtual void updateContentsScale(int w, int h) override;

    PublishGLImage* _timerImage;
    int _seconds;
    int _timerId;
};

#endif // OVERLAYTIMER_H
