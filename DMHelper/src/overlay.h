#ifndef OVERLAY_H
#define OVERLAY_H

#include <QObject>
#include <QSize>
#include <QOpenGLFunctions>

class Campaign;

class Overlay : public QObject
{
    Q_OBJECT
public:
    explicit Overlay(QObject *parent = nullptr);

    virtual int getOverlayType() const = 0;

    void setCampaign(Campaign* campaign);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix);

public slots:
    virtual void recreateContents();

signals:
    void triggerUpdate();

protected:
    virtual void doSetCampaign(Campaign* campaign);
    virtual void doInitializeGL();
    virtual void doResizeGL(int w, int h);
    virtual void doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix);

    virtual void createContentsGL() = 0;
    virtual void updateContentsScale(int w, int h);

    Campaign* _campaign;
    bool _recreateContents;
};

#endif // OVERLAY_H
