#include "overlay.h"
#include <QDomElement>

Overlay::Overlay(QObject *parent) :
    DMHObjectBase{parent},
    _visible(true),
    _scale(1.0),
    _opacity(100),
    _campaign(nullptr),
    _recreateContents(false),
    _updateContents(false)
{
}

QDomElement Overlay::outputXML(QDomDocument &doc, QDomElement &parentElement, QDir& targetDirectory, bool isExport)
{
    // TODO
    return QDomElement();
}

void Overlay::inputXML(const QDomElement &element, bool isImport)
{
    // TODO
}

void Overlay::postProcessXML(Campaign* campaign, const QDomElement &element, bool isImport)
{
    // TODO
}

bool Overlay::isInitialized() const
{
    return _campaign != nullptr;
}

bool Overlay::isVisible() const
{
    return _visible;
}

qreal Overlay::getScale() const
{
    return _scale;
}

int Overlay::getOpacity() const
{
    return _opacity;
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
        _updateContents = false;
    }
    else if(_updateContents)
    {
        _updateContents = false;
    }

    doPaintGL(functions, targetSize, modelMatrix);
}

void Overlay::recreateContents()
{
    _recreateContents = true;
    emit triggerUpdate();
}

void Overlay::updateContents()
{
    _updateContents = true;
    emit triggerUpdate();
}

void Overlay::setVisible(bool visible)
{
    if(_visible == visible)
        return;

    _visible = visible;
    emit dirty();
}

void Overlay::setScale(qreal scale)
{
    if(qFuzzyCompare(_scale, scale))
        return;

    _scale = scale;
    emit dirty();
}

void Overlay::setOpacity(int opacity)
{
    if(_opacity == opacity)
        return;

    _opacity = opacity;
    emit dirty();
}

void Overlay::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    // TODO
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

void Overlay::updateContentsGL()
{
}

void Overlay::updateContentsScale(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}
