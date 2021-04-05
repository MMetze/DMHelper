#include "encountertextrenderer.h"
#include "dmconstants.h"
#include <QTextDocument>
#include <QPainter>
#include <QDebug>

EncounterTextRenderer::EncounterTextRenderer(EncounterText& text, QImage backgroundImage, QObject *parent) :
    CampaignObjectRenderer(parent),
    _text(text),
    _targetSize(),
    _rotation(0),
    _animationRunning(false),
    _textPos(),
    _elapsed(),
    _timerId(0),
    _backgroundImage(backgroundImage),
    _prescaledImage(),
    _textImage(),
    _videoPlayer(nullptr)
{
}

EncounterTextRenderer::~EncounterTextRenderer()
{
    stopPublishTimer();

    if(_videoPlayer)
    {
        VideoPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        delete deletePlayer;
    }
}

void EncounterTextRenderer::startRendering()
{
    qDebug() << "[EncounterTextRenderer] Starting renderer. Animated: " << _text.getAnimated() << ", running: " << _animationRunning << ", background image: " << _backgroundImage.size() << ", file: " << _text.getImageFile();

    if(_text.getAnimated())
    {
        if(_animationRunning)
            return;

        emit showPublishWindow();
        prepareImages();

        if(isVideo())
            createVideoPlayer();

        emit animationStarted();
        startPublishTimer();

        _animationRunning = true;
    }
    else
    {
        if((isVideo()) && (_animationRunning))
            return;

        emit showPublishWindow();
        rewind();
        prepareImages();
        if(!isVideo())
        {
            QImage imagePublish = _prescaledImage;
            drawTextImage(&imagePublish);
            emit publishImage(imagePublish);
        }
        else
        {
            stopPublishTimer();
            createVideoPlayer();
            emit animationStarted();
            startPublishTimer();

            _animationRunning = true;
        }
    }
}

void EncounterTextRenderer::stopRendering()
{
    qDebug() << "[EncounterTextRenderer] Stopping renderer. Animated: " << _text.getAnimated() << ", running: " << _animationRunning << ", background image: " << _backgroundImage.size() << ", file: " << _text.getImageFile();

    if(_text.getAnimated())
    {
        if(!_animationRunning)
            return;

        stopPublishTimer();
        _animationRunning = false;
    }
    else
    {
        if((isVideo()) && (!_animationRunning))
            return;

        emit showPublishWindow();
        rewind();
        prepareImages();
        if(!isVideo())
        {
            QImage imagePublish = _prescaledImage;
            drawTextImage(&imagePublish);
            emit publishImage(imagePublish);
        }
        else
        {
            stopPublishTimer();
            _animationRunning = false;
        }
    }
}

void EncounterTextRenderer::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
    {
        int oldHeight = _targetSize.height();
        _targetSize = newSize;
        if(isAnimated())
        {
            prepareImages();

            if((_text.getAnimated()) && (oldHeight > 0) && (newSize.height() > 0))
                _textPos.setY(_textPos.y() * newSize.height() / oldHeight);
        }

        if(_videoPlayer)
            _videoPlayer->targetResized(newSize);
    }
}

void EncounterTextRenderer::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
    if(_animationRunning)
    {
        stopPublishTimer();
        prepareImages();
        startPublishTimer();
    }
}

void EncounterTextRenderer::rewind()
{
    qreal yPos = 0.0;
    if(_text.getAnimated())
    {
        if(_rotation % 180 == 0)
        {
            if(_prescaledImage.height() > 0)
                yPos = _prescaledImage.height();
        }
        else
        {
            if(_prescaledImage.width() > 0)
                yPos = _prescaledImage.width();
        }
    }
    else
    {
        _textPos.setY(0);
    }

    _textPos.setY(yPos);
}

void EncounterTextRenderer::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    qreal elapsedtime = 0.0;
    if(_text.getAnimated())
    {
        elapsedtime = _elapsed.restart();
        _textPos.ry() -= _text.getScrollSpeed() * (_prescaledImage.height() / 250) * (elapsedtime / 1000.0);
    }

    if(_textImage.isNull())
        prepareTextImage();

    QImage targetImage;
    if(_videoPlayer)
    {
        if((!_animationRunning) ||
           (!_videoPlayer->isNewImage()) ||
           (!_videoPlayer->getImage()) || (_videoPlayer->getImage()->isNull()) ||
           (_videoPlayer->isError()) ||
           (!_videoPlayer->getMutex()))
            return;

        QMutexLocker locker(_videoPlayer->getMutex());
        if(_rotation != 0)
            targetImage = _videoPlayer->getImage()->transformed(QTransform().rotate(_rotation), Qt::FastTransformation);
        else
            targetImage = *(_videoPlayer->getImage());
    }
    else
    {
        targetImage = _prescaledImage;
    }

    drawTextImage(&targetImage);
    emit animateImage(targetImage);

    if((_text.getAnimated()) && (_textPos.y() < -_textImage.height()))
        emit animationStopped();
}

void EncounterTextRenderer::startPublishTimer()
{
    if(!_timerId)
    {
        rewind();
        if(_text.getAnimated())
            _elapsed.start();
        _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
    }
}

void EncounterTextRenderer::stopPublishTimer()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
}

void EncounterTextRenderer::prepareImages()
{
    if(_backgroundImage.isNull())
    {
        _prescaledImage = QImage(_targetSize, QImage::Format_ARGB32_Premultiplied);
        _prescaledImage.fill(QColor(0, 0, 0, 0));
    }
    else
    {
        _prescaledImage = _backgroundImage.scaled(getRotatedTargetSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if(_rotation != 0)
            _prescaledImage = _prescaledImage.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
    }

    _textImage = QImage();
    prepareTextImage();
}

void EncounterTextRenderer::prepareTextImage()
{
    if(_prescaledImage.isNull())
        return;

    QTextDocument doc;
    doc.setHtml(_text.getTranslated() ? _text.getTranslatedText() : _text.getText());

    QSize rotatedSize = getRotatedSize(_prescaledImage.size());

    int absoluteWidth = rotatedSize.width() * _text.getTextWidth() / 100;
    int targetMargin = (rotatedSize.width() - absoluteWidth) / 2;

    doc.setTextWidth(absoluteWidth);

    _textImage = QImage(rotatedSize.width(), doc.size().height(), QImage::Format_ARGB32_Premultiplied);
    _textImage.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&_textImage);
        painter.translate(targetMargin, 0);
        doc.drawContents(&painter);
    painter.end();

    _textImage = _textImage.scaledToWidth(rotatedSize.width(), Qt::SmoothTransformation);
    if(_rotation != 0)
        _textImage = _textImage.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
}

void EncounterTextRenderer::drawTextImage(QPaintDevice* target)
{
    if(!target)
        return;

    QPainter painter(target);
    QPointF drawPoint(0.0, 0.0);

    if(_rotation == 0)
        drawPoint = QPointF(0.0, _textPos.y());
    else if(_rotation == 90)
        drawPoint = QPointF(_prescaledImage.width() - _textImage.width() - _textPos.y(), 0.0);
    else if(_rotation == 180)
        drawPoint = QPointF(0.0, _prescaledImage.height() - _textImage.height() - _textPos.y());
    else if(_rotation == 270)
        drawPoint = QPointF(_textPos.y(), 0.0);

    painter.drawImage(drawPoint, _textImage, _textImage.rect());
}

void EncounterTextRenderer::createVideoPlayer()
{
    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    qDebug() << "[ScrollingText] Publish FoW Player animation started";

    // TODO: consider audio in the scrolling text
    QSize rotatedSize = getRotatedTargetSize();
    _videoPlayer = new VideoPlayer(_text.getImageFile(), rotatedSize, true, false);
}

bool EncounterTextRenderer::isVideo() const
{
    return((_backgroundImage.isNull()) && (!_text.getImageFile().isEmpty()));
}

bool EncounterTextRenderer::isAnimated() const
{
    return((isVideo()) || (_text.getAnimated()));
}

QSize EncounterTextRenderer::getRotatedTargetSize()
{
    return getRotatedSize(_targetSize);
}

QSize EncounterTextRenderer::getRotatedSize(const QSize& size)
{
    return (_rotation % 180 == 0) ? size : size.transposed();
}
