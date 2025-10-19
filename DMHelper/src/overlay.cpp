#include "overlay.h"
#include <QDomElement>

Overlay::Overlay(const QString& name, QObject *parent) :
    CampaignObjectBase{name, parent},
    _visible(true),
    _scale(1.0),
    _opacity(100),
    _campaign(nullptr),
    _recreateContents(false),
    _updateContents(false)
{
}

void Overlay::inputXML(const QDomElement &element, bool isImport)
{
    setVisible(static_cast<bool>(element.attribute(QString("visible"), QString::number(1)).toInt()));
    setScale(element.attribute(QString("scale"), QString::number(1.0, 'g', 1)).toDouble());
    setOpacity(element.attribute(QString("opacity"), QString::number(100)).toInt());

    CampaignObjectBase::inputXML(element, isImport);
}

void Overlay::copyValues(const CampaignObjectBase* other)
{
    const Overlay* otherOverlay = dynamic_cast<const Overlay*>(other);
    if(!otherOverlay)
        return;

    _visible = otherOverlay->_visible;
    _scale = otherOverlay->_scale;
    _opacity = otherOverlay->_opacity;

    CampaignObjectBase::copyValues(other);
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

QDomElement Overlay::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("overlay");
}

void Overlay::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute(QString("visible"), _visible);
    element.setAttribute(QString("scale"), QString::number(_scale, 'g', 6));
    element.setAttribute(QString("opacity"), _opacity);

    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);
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
