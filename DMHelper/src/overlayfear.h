#ifndef OVERLAYFEAR_H
#define OVERLAYFEAR_H

#include <QObject>
#include "overlay.h"

class PublishGLImage;

class OverlayFear : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayFear(QObject *parent = nullptr);

    virtual int getOverlayType() const override;

protected:
    virtual void doSetCampaign(Campaign* campaign) override;
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix) override;

    virtual void createContentsGL() override;
    virtual void updateContentsScale(int w, int h) override;

    PublishGLImage* _fearCounter;

};

#endif // OVERLAYFEAR_H
