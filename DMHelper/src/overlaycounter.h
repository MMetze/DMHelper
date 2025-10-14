#ifndef OVERLAYCOUNTER_H
#define OVERLAYCOUNTER_H

#include <QObject>
#include "overlay.h"

class PublishGLImage;

class OverlayCounter : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayCounter(int counter, QObject *parent = nullptr);

    virtual int getOverlayType() const override;

    virtual int getCounterValue() const;

public slots:
    virtual void setCounterValue(int value);
    virtual void increase();
    virtual void decrease();

protected:
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;

    virtual void createContentsGL() override;
    virtual void updateContentsScale(int w, int h) override;

    PublishGLImage* _counterImage;
    int _counter;
};

#endif // OVERLAYCOUNTER_H
