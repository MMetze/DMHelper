#include "overlay.h"
#include <QDomElement>
#include <QImage>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QDir>

const qreal OVERLAY_WINDOW_SCALE = 0.1;

Overlay::Overlay(const QString& name, QObject *parent) :
    Popup{parent},
    _visible(true),
    _scale(OVERLAY_WINDOW_SCALE),
    _opacity(100),
    _campaign(nullptr),
    _recreateContents(false),
    _updateContents(false),
    _initialized(false)
{
    setObjectName(name);
}

bool Overlay::isDMOnly() const
{
    return false;
}

void Overlay::inputXML(const QDomElement &element)
{
    setObjectName(element.attribute(QString("name"), QString("Overlay")));
    setVisible(static_cast<bool>(element.attribute(QString("visible"), QString::number(1)).toInt()));
    setScale(element.attribute(QString("scale"), QString::number(OVERLAY_WINDOW_SCALE, 'g', 1)).toDouble());
    setOpacity(element.attribute(QString("opacity"), QString::number(100)).toInt());
}

QDomElement Overlay::outputXML(QDomDocument &doc, QDomElement &parent, QDir& targetDirectory)
{
    QDomElement element = doc.createElement(QString("overlay"));

    element.setAttribute(QString("name"), objectName());
    element.setAttribute(QString("type"), getOverlayType());
    element.setAttribute(QString("visible"), isVisible() ? 1 : 0);
    element.setAttribute(QString("scale"), QString::number(getScale(), 'g', 1));
    element.setAttribute(QString("opacity"), QString::number(getOpacity()));

    internalOutputXML(doc, element, targetDirectory);

    parent.appendChild(element);
    return element;
}

bool Overlay::isInitialized() const
{
    return _initialized;
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

QSize Overlay::getSize() const
{
    return QSize();
}

void Overlay::setCampaign(Campaign* campaign)
{
    doSetCampaign(campaign);
    _campaign = campaign;
    recreateContents();
}

void Overlay::initializeGL()
{
    doInitializeGL();
    _recreateContents = true;
    _initialized = true;
}

void Overlay::resizeGL(int w, int h)
{
    doResizeGL(w, h);
    setX(w - getSize().width());
}

void Overlay::paintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix, int yOffset)
{
    if(_recreateContents)
    {
        createContentsGL();
        doResizeGL(targetSize.width(), targetSize.height());
        setX(targetSize.width() - getSize().width());
        _recreateContents = false;
        _updateContents = false;
    }
    else if(_updateContents)
    {
        updateContentsGL();
        _updateContents = false;
    }

    setY(targetSize.height() - getSize().height() - yOffset);
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
    emit triggerUpdate();
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

void Overlay::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory)
{
    Q_UNUSED(doc);
    Q_UNUSED(element);
    Q_UNUSED(targetDirectory);
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

QImage Overlay::textToImage(const QString& text)
{
    QFont f;
    f.setPixelSize(256);
    f.setStyleStrategy(QFont::ForceOutline);

    // Build text path at origin
    QPainterPath path;
    path.addText(0, 0, f, text);

    // Measure it
    QRectF bounds = path.boundingRect();

    // Create an image just large enough
    QImage resultImage(bounds.size().toSize().grownBy(QMargins(4, 4, 4, 4)), QImage::Format_ARGB32_Premultiplied);
    resultImage.fill(Qt::transparent);

    // Prepare painter
    QPainter p(&resultImage);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, 5));
        p.setBrush(QColor(115, 18, 0));

        // Translate so that text fits fully inside the image (since bounds may start <0)
        p.translate(-bounds.topLeft() + QPointF(2.f, 2.f));

        // Draw it
        p.drawPath(path);
    p.end();

    return resultImage;
}
