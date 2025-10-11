#include "overlay.h"

Overlay::Overlay(QObject *parent) :
    QObject{parent},
    _campaign(nullptr),
    _recreateContents(false)
{
}

void Overlay::setCampaign(Campaign* campaign)
{
    doSetCampaign(campaign);
    _campaign = campaign;
    recreateContents();
}

void Overlay::initializeGL()
{
    recreateContents();
    doInitializeGL();
}

void Overlay::resizeGL(int w, int h)
{
    updateContentsScale(w, h);
    doResizeGL(w, h);
}

void Overlay::paintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    if(_recreateContents)
    {
        createContentsGL();
        updateContentsScale(targetSize.width(), targetSize.height());
        _recreateContents = false;
    }

    doPaintGL(functions, targetSize, modelMatrix);
}

void Overlay::recreateContents()
{
    _recreateContents = true;
    emit triggerUpdate();
}

void Overlay::doSetCampaign(Campaign* campaign)
{
    Q_UNUSED(campaign);
}

void Overlay::doInitializeGL()
{
}

void Overlay::doResizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void Overlay::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(functions);
    Q_UNUSED(targetSize);
    Q_UNUSED(modelMatrix);
}

void Overlay::updateContentsScale(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}
