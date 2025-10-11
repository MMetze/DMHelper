#include "overlaytimer.h"
#include "publishglimage.h"
#include "dmconstants.h"
#include <QPainter>
#include <QPainterPath>

OverlayTimer::OverlayTimer(int seconds, QObject *parent) :
    Overlay{parent},
    _timerImage(nullptr),
    _seconds(seconds),
    _timerId(0)
{
}

int OverlayTimer::getOverlayType() const
{
    return DMHelper::OverlayType_Timer;
}

void OverlayTimer::setTimerValue(int seconds)
{
    _seconds = seconds;
    emit triggerUpdate();
}

int OverlayTimer::getTimerValue() const
{
    return _seconds;
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

    recreateContents();
}

void OverlayTimer::doPaintGL(QOpenGLFunctions *functions, QSize targetSize, int modelMatrix)
{
    Q_UNUSED(targetSize);

    if((!_timerImage) || (!functions))
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerImage->getMatrixData(), _timerImage->getMatrix());
    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _timerImage->getMatrixData());
    _timerImage->paintGL(functions, nullptr);
}

void OverlayTimer::createContentsGL()
{
    if(_timerImage)
    {
        delete _timerImage;
        _timerImage = nullptr;
    }

    if(!_campaign)
        return;

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
    QImage timerImage(bounds.size().toSize().grownBy(QMargins(4, 4, 4, 4)), QImage::Format_ARGB32_Premultiplied);
    timerImage.fill(Qt::transparent);

    // Prepare painter
    QPainter p(&timerImage);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::white, 5));
    p.setBrush(QColor(115, 18, 0));

    // Translate so that text fits fully inside the image (since bounds may start <0)
    p.translate(-bounds.topLeft() + QPointF(2.f, 2.f));

    // Draw it
    p.drawPath(path);
    p.end();


    _timerImage = new PublishGLImage(timerImage, false);
}

void OverlayTimer::updateContentsScale(int w, int h)
{
    if(!_timerImage)
        return;

    qreal tokenHeight = static_cast<qreal>(h) / 10.0;
    _timerImage->setScale(tokenHeight / static_cast<qreal>(_timerImage->getImageSize().height()));
    _timerImage->setX(static_cast<qreal>(w) - _timerImage->getSize().width());
    _timerImage->setY(static_cast<qreal>(h) - (_timerImage->getSize().height() * 3));
}
