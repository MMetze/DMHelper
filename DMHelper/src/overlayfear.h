#ifndef OVERLAYFEAR_H
#define OVERLAYFEAR_H

#include <QObject>
#include "overlay.h"

class PublishGLImage;

class OverlayFear : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayFear(const QString& name = QString("Fear"), QObject *parent = nullptr);

    // From Overlay
    virtual int getOverlayType() const override;
    virtual QSize getSize() const override;
    virtual void prepareFrame(OverlayFrame* frame) override;

public slots:
    // From Overlay
    virtual void setX(int x) override;
    virtual void setY(int y) override;

protected:
    // From Overlay
    virtual void doSetCampaign(Campaign* campaign) override;
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;
    virtual void doResizeGL(int w, int h) override;

    virtual void createContentsGL() override;

    PublishGLImage* _fearCounter;

};

#endif // OVERLAYFEAR_H
