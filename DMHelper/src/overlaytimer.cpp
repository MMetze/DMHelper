#include "overlaytimer.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include <QDomElement>
#include <QPainter>
#include <QPainterPath>

OverlayTimer::OverlayTimer(int seconds, const QString& name, QObject *parent) :
    Overlay{name, parent},
    _timerPublishImage(nullptr),
    _timerImage(),
    _seconds(seconds),
    _timerId(0)
{
}

void OverlayTimer::inputXML(const QDomElement &element, bool isImport)
{
    setTimerValue(element.attribute(QString("seconds"), QString::number(0)).toInt());

    Overlay::inputXML(element, isImport);
}

void OverlayTimer::copyValues(const CampaignObjectBase* other)
{
    const OverlayTimer* otherOverlayTimer = dynamic_cast<const OverlayTimer*>(other);
    if(!otherOverlayTimer)
        return;

    _seconds = otherOverlayTimer->_seconds;

    Overlay::copyValues(other);
}

int OverlayTimer::getOverlayType() const
{
    return DMHelper::OverlayType_Timer;
}

int OverlayTimer::getTimerValue() const
{
    return _seconds;
}

void OverlayTimer::setTimerValue(int seconds)
{
    if(_seconds == seconds)
        return;

    _seconds = seconds;
    emit triggerUpdate();
}

void OverlayTimer::toggle(bool play)
{
    if(play)
        start();
    else
        stop();
}

void OverlayTimer::start()
{
    if(_timerId > 0)
        return;

    _timerId = startTimer(1000);
}

void OverlayTimer::stop()
{
    if(_timerId == 0)
        return;

    killTimer(_timerId);
    _timerId = 0;
}

void OverlayTimer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(_timerId == 0)
        return;

    if(--_seconds <= 0)
        stop();

    updateContents();
}

void OverlayTimer::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute(QString("seconds"), QString::number(_seconds));

    Overlay::internalOutputXML(doc, element, targetDirectory, isExport);
}

void OverlayTimer::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_timerPublishImage) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerPublishImage->getMatrixData(), _timerPublishImage->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerPublishImage->getMatrixData());
    _timerPublishImage->paintGL(functions, nullptr);
}

void OverlayTimer::createContentsGL()
{
    if(_timerPublishImage)
    {
        delete _timerPublishImage;
        _timerPublishImage = nullptr;
    }

    _timerImage = QImage();
    createTimerImage();
    _timerPublishImage = new PublishGLImage(_timerImage, false);
}

void OverlayTimer::updateContentsGL()
{
    if(!_timerPublishImage)
        return;

    createTimerImage();
    _timerPublishImage->updateImage(_timerImage);
}

void OverlayTimer::updateContentsScale(int w, int h)
{
    if(!_timerPublishImage)
        return;

    qreal tokenHeight = static_cast<qreal>(h) / 10.0;
    _timerPublishImage->setScale(tokenHeight / static_cast<qreal>(_timerPublishImage->getImageSize().height()));
    _timerPublishImage->setX(static_cast<qreal>(w) - _timerPublishImage->getSize().width());
    _timerPublishImage->setY(static_cast<qreal>(h) - (_timerPublishImage->getSize().height() * 3));
}

void OverlayTimer::createTimerImage()
{
    QFont f;
    f.setPixelSize(256);
    f.setStyleStrategy(QFont::ForceOutline);

    // Build text path at origin
    QPainterPath path;
    QString timerString = QString::number(_seconds / 60) + QString(":");
    int modSeconds = _seconds % 60;
    if(modSeconds < 10)
        timerString += QString("0");
    timerString += QString::number(modSeconds);
    path.addText(0, 0, f, timerString);

    // Measure it
    QRectF bounds = path.boundingRect();

    // Create an image just large enough
    if(_timerImage.isNull())
        _timerImage = QImage(bounds.size().toSize().grownBy(QMargins(4, 4, 4, 4)), QImage::Format_ARGB32_Premultiplied);
    _timerImage.fill(Qt::transparent);

    // Prepare painter
    QPainter p(&_timerImage);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::white, 5));
    p.setBrush(QColor(115, 18, 0));

    // Translate so that text fits fully inside the image (since bounds may start <0)
    p.translate(-bounds.topLeft() + QPointF(2.f, 2.f));

    // Draw it
    p.drawPath(path);
    p.end();
}
