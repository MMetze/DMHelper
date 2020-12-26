#include "encounterscrollingtextedit.h"
#include "ui_encounterscrollingtextedit.h"
#include "encounterscrollingtext.h"
#include "scrollingtextwindow.h"
#include "dmconstants.h"
#include "texteditmargins.h"
#include <QFontDatabase>
#include <QFile>
#include <QFileDialog>
#include <QPainter>
#include <QTextCursor>
#include <QTextBlockFormat>
#include <QMessageBox>
#include <QDebug>

EncounterScrollingTextEdit::EncounterScrollingTextEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::EncounterScrollingTextEdit),
    _scrollingText(nullptr),
    _backgroundWidth(0),
    _backgroundImg(),
    _backgroundImgScaled(),
    _prescaledImg(),
    _textImg(),
    _textPos(),
    _startPos(0),
    _targetSize(),
    _elapsed(),
    _timerId(0),
    _videoPlayer(nullptr),
    _isDMPlayer(false),
    _backgroundVideo(),
    _animationRunning(false),
    _firstTime(true),
    _rotation(0)
{
    ui->setupUi(this);

    connect(ui->edtText,SIGNAL(textChanged()),this,SLOT(setPlainText()));

    ui->edtText->installEventFilter(this);

}

EncounterScrollingTextEdit::~EncounterScrollingTextEdit()
{
    stopPublishTimer();

    VideoPlayer* deletePlayer = _videoPlayer;
    _videoPlayer = nullptr;
    delete deletePlayer;

    delete ui;
}

void EncounterScrollingTextEdit::activateObject(CampaignObjectBase* object)
{
    EncounterScrollingText* scrollingText = dynamic_cast<EncounterScrollingText*>(object);
    if(!scrollingText)
        return;

    if(_scrollingText != nullptr)
    {
        qDebug() << "[EncounterScrollingTextEdit] ERROR: New scrolling text object activated without deactivating the existing scrolling text object first!";
        deactivateObject();
    }

    setScrollingText(scrollingText);

    emit checkableChanged(true);
    emit setPublishEnabled(true);
}

void EncounterScrollingTextEdit::deactivateObject()
{
    if(!_scrollingText)
    {
        qDebug() << "[EncounterScrollingTextEdit] WARNING: Invalid (nullptr) scrolling text object deactivated!";
        return;
    }

    unsetScrollingText(_scrollingText);
}

EncounterScrollingText* EncounterScrollingTextEdit::getScrollingText() const
{
    return _scrollingText;
}

void EncounterScrollingTextEdit::setScrollingText(EncounterScrollingText* scrollingText)
{
    if(_scrollingText == scrollingText)
        return;

    qDebug() << "[Scrolling Text] Setting scrolling text to: " << scrollingText;

    if(_videoPlayer)
        cleanupPlayer();

    _scrollingText = scrollingText;

    if(!scrollingText)
        return;

    emit scrollSpeedChanged(_scrollingText->getScrollSpeed());
    emit imageWidthChanged(_scrollingText->getImageWidth());
    emit imageFileChanged(_scrollingText->getImageFile());
    emit fontFamilyChanged(_scrollingText->getFontFamily());
    emit fontSizeChanged(_scrollingText->getFontSize());
    emit fontBoldChanged(_scrollingText->getFontBold());
    emit fontItalicsChanged(_scrollingText->getFontItalics());
    emit alignmentChanged(_scrollingText->getAlignment());
    emit colorChanged(_scrollingText->getFontColor());

    ui->edtText->setPlainText(_scrollingText->getText());
    ui->edtText->setTextWidth(_scrollingText->getImageWidth());

    setTextFont();
    setTextAlignment();
    setTextColor();
    loadImage();

    _firstTime = true;

    connect(scrollingText, SIGNAL(textChanged(const QString&)), this, SIGNAL(textChanged(const QString&)));
    //connect(scrollingText, SIGNAL(textChanged(const QString&)), this, SLOT(setTextFont()));
    //connect(scrollingText, SIGNAL(textChanged(const QString&)), this, SLOT(setTextAlignment()));
    //connect(scrollingText, SIGNAL(textChanged(const QString&)), this, SLOT(setTextColor()));
    connect(scrollingText, SIGNAL(textChanged(const QString&)), this, SLOT(setAllFormats()));
    connect(scrollingText, SIGNAL(scrollSpeedChanged(int)), this, SIGNAL(scrollSpeedChanged(int)));
    connect(scrollingText, SIGNAL(imageFileChanged(const QString&)), this, SIGNAL(imageFileChanged(const QString&)));
    connect(scrollingText, SIGNAL(imageFileChanged(const QString&)), this, SLOT(loadImage()));
    connect(scrollingText, SIGNAL(fontFamilyChanged(const QString&)), this, SIGNAL(fontFamilyChanged(const QString&)));
    connect(scrollingText, SIGNAL(fontFamilyChanged(const QString&)), this, SLOT(setTextFont()));
    connect(scrollingText, SIGNAL(fontSizeChanged(int)), this, SIGNAL(fontSizeChanged(int)));
    connect(scrollingText, SIGNAL(fontSizeChanged(int)), this, SLOT(setTextFont()));
    connect(scrollingText, SIGNAL(fontBoldChanged(bool)), this, SIGNAL(fontBoldChanged(bool)));
    connect(scrollingText, SIGNAL(fontBoldChanged(bool)), this, SLOT(setTextFont()));
    connect(scrollingText, SIGNAL(fontItalicsChanged(bool)), this, SIGNAL(fontItalicsChanged(bool)));
    connect(scrollingText, SIGNAL(fontItalicsChanged(bool)), this, SLOT(setTextFont()));
    connect(scrollingText, SIGNAL(alignmentChanged(Qt::Alignment)), this, SIGNAL(alignmentChanged(Qt::Alignment)));
    connect(scrollingText, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(setTextAlignment()));
    connect(scrollingText, SIGNAL(imageWidthChanged(int)), this, SIGNAL(imageWidthChanged(int)));
    connect(scrollingText, &EncounterScrollingText::imageWidthChanged, ui->edtText, &TextEditMargins::setTextWidth);
    connect(scrollingText, SIGNAL(fontColorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));
    connect(scrollingText, SIGNAL(fontColorChanged(QColor)), this, SLOT(setTextColor()));
}

void EncounterScrollingTextEdit::unsetScrollingText(EncounterScrollingText* scrollingText)
{
    Q_UNUSED(scrollingText);

    qDebug() << "[Scrolling Text] Unsetting scrolling text...";

    if(scrollingText != _scrollingText)
        qDebug() << "[Scrolling Text] WARNING: unsetting scrolling text with a DIFFERENT encounter than currently set! Current: " << QString(_scrollingText ? _scrollingText->getID().toString() : "nullptr") << ", Unset: " << QString(scrollingText ? scrollingText->getID().toString() : "nullptr");

    stopAnimation();
    cleanupPlayer();

    if(_scrollingText)
        disconnect(_scrollingText, nullptr, this, nullptr);

    _scrollingText = nullptr;

    _backgroundImg = QImage();
    _backgroundImgScaled = QImage();
    ui->edtText->clear();
}

void EncounterScrollingTextEdit::setScrollSpeed(int scrollSpeed)
{
    if(_scrollingText)
        _scrollingText->setScrollSpeed(scrollSpeed);
}

void EncounterScrollingTextEdit::setImageFile(const QString& imgFile)
{
    if(_scrollingText)
        _scrollingText->setImageFile(imgFile);
}

void EncounterScrollingTextEdit::browseImageFile()
{
    QString imageFileName = QFileDialog::getOpenFileName(this,QString("Select Image File"));

    if((imageFileName.isEmpty()) || (!QFile::exists(imageFileName)))
    {
        qDebug() << "[ScrollingText] Invalid image file selected for animation";
        return;
    }

    setImageFile(imageFileName);
}

void EncounterScrollingTextEdit::setText(const QString& newText)
{
    if(_scrollingText)
        _scrollingText->setText(newText);
}

void EncounterScrollingTextEdit::setFontFamily(const QString& fontFamily)
{
    if(_scrollingText)
        _scrollingText->setFontFamily(fontFamily);
}

void EncounterScrollingTextEdit::setFontSize(int fontSize)
{
    if(_scrollingText)
        _scrollingText->setFontSize(fontSize);
}

void EncounterScrollingTextEdit::setFontBold(bool fontBold)
{
    if(_scrollingText)
        _scrollingText->setFontBold(fontBold);
}

void EncounterScrollingTextEdit::setFontItalics(bool fontItalics)
{
    if(_scrollingText)
        _scrollingText->setFontItalics(fontItalics);
}

void EncounterScrollingTextEdit::setAlignment(Qt::Alignment alignment)
{
    if(_scrollingText)
        _scrollingText->setAlignment(alignment);
}

void EncounterScrollingTextEdit::setImageWidth(int imageWidth)
{
    if(_scrollingText)
        _scrollingText->setImageWidth(imageWidth);
}

void EncounterScrollingTextEdit::setColor(QColor color)
{
    if(_scrollingText)
        _scrollingText->setFontColor(color);
}

void EncounterScrollingTextEdit::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
    {
        _targetSize = newSize;
        prepareImages();
        if((_videoPlayer) && (_animationRunning))
        {
            _videoPlayer->targetResized(newSize);
        }
    }
}

void EncounterScrollingTextEdit::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(!_scrollingText)
        return;

    qreal elapsedtime = _elapsed.restart();
    _textPos.ry() -= _scrollingText->getScrollSpeed() * elapsedtime / 1000.0;

    if(_textImg.isNull())
        prepareTextImage();

    QImage targetImg;
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
        {
            targetImg = _videoPlayer->getImage()->transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
        }
        else
        {
            targetImg = *(_videoPlayer->getImage());
        }
    }
    else
    {
        targetImg = _prescaledImg;
    }
    QPainter painter(&targetImg);
    QPointF drawPoint = _textPos.toPoint();
    if(_rotation == 90)
    {
        drawPoint = QPointF(-drawPoint.y(), drawPoint.x());
    }
    else if(_rotation == 180)
    {
        drawPoint.setY(-drawPoint.y());
    }
    else if(_rotation == 270)
    {
        drawPoint = QPointF(drawPoint.y(), drawPoint.x());
    }
    painter.drawImage(drawPoint, _textImg, _textImg.rect());
    emit animateImage(targetImg);
}

bool EncounterScrollingTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if((ui->edtText) && (watched == ui->edtText))
    {
        if(event->type() == QEvent::Paint)
        {
            if(!_backgroundImgScaled.isNull())
            {
                QPainter paint(ui->edtText);
                paint.drawImage(0, 0, _backgroundImgScaled);
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void EncounterScrollingTextEdit::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if(!_backgroundImg.isNull())
    {
        _backgroundImgScaled = _backgroundImg.scaled(ui->edtText->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
    else if(!_backgroundVideo.isNull())
    {
        _backgroundImgScaled = _backgroundVideo.scaled(ui->edtText->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
}

void EncounterScrollingTextEdit::createVideoPlayer(bool dmPlayer)
{
    if(!_scrollingText)
        return;

    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }

    _isDMPlayer = dmPlayer;

    if(dmPlayer)
    {
        qDebug() << "[ScrollingText] Publish FoW DM animation started";
        _videoPlayer = new VideoPlayer(_scrollingText->getImageFile(), QSize(0, 0), true, false);
        if(_videoPlayer->isNewImage())
            updateVideoBackground();
        else
            connect(_videoPlayer, SIGNAL(screenShotAvailable()), this, SLOT(updateVideoBackground()));

    }
    else
    {
        qDebug() << "[ScrollingText] Publish FoW Player animation started";

        // TODO: consider audio in the scrolling text
        QSize rotatedSize = getRotatedTargetSize();
        _videoPlayer = new VideoPlayer(_scrollingText->getImageFile(), rotatedSize, true, false);
    }
}

void EncounterScrollingTextEdit::cleanupPlayer()
{
    stopPublishTimer();
    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }
}

void EncounterScrollingTextEdit::setPlainText()
{
    setText(ui->edtText->toPlainText());
}

void EncounterScrollingTextEdit::setAllFormats()
{
    if(!_scrollingText)
        return;

    QTextCursor cursor(ui->edtText->textCursor());

    ui->edtText->selectAll();

    setTextFontFormat();
    setTextAlignmentFormat();
    setTextColorFormat();

    ui->edtText->setTextCursor(cursor);
}

void EncounterScrollingTextEdit::setTextFont()
{
    if(!_scrollingText)
        return;

    ui->edtText->selectAll();
    setTextFontFormat();
    moveCursorToEnd();
}

void EncounterScrollingTextEdit::setTextAlignment()
{
    if(!_scrollingText)
        return;

    ui->edtText->selectAll();
    setTextAlignmentFormat();
    moveCursorToEnd();

    emit alignmentChanged(getAlignment());
}

void EncounterScrollingTextEdit::setTextColor()
{
    if(!_scrollingText)
        return;

    // Set the text color for all text then move the cursor to the end
    ui->edtText->selectAll();
    setTextColorFormat();
    moveCursorToEnd();

    emit colorChanged(_scrollingText->getFontColor());
}

void EncounterScrollingTextEdit::setTextFontFormat()
{
    if(!_scrollingText)
        return;

    ui->edtText->setFontFamily(_scrollingText->getFontFamily());
    ui->edtText->setFontItalic(_scrollingText->getFontItalics());
    ui->edtText->setFontWeight(_scrollingText->getFontBold() ? QFont::Bold : QFont::Normal);
    ui->edtText->setFontPointSize(_scrollingText->getFontSize());
}

void EncounterScrollingTextEdit::setTextAlignmentFormat()
{
    if(!_scrollingText)
        return;

    ui->edtText->setAlignment(getAlignment());
}

void EncounterScrollingTextEdit::setTextColorFormat()
{
    if(!_scrollingText)
        return;

    ui->edtText->setTextColor(_scrollingText->getFontColor());
}

void EncounterScrollingTextEdit::publishClicked(bool checked)
{
    if(_animationRunning == checked)
        return;

    _animationRunning = checked;

    if(!_backgroundImg.isNull())
    {
        if(_animationRunning)
        {
            prepareImages();

            emit animationStarted();
            emit showPublishWindow();
            // TODO: add music

            startPublishTimer();
        }
        else
        {
            stopPublishTimer();
        }
    }
    else if((_scrollingText) && (!_scrollingText->getImageFile().isEmpty()))
    {
        stopPublishTimer();
        if(_animationRunning)
        {
            prepareImages();
            createVideoPlayer(false);

            //emit animationStarted(ui->framePublish->getColor());
            emit animationStarted();
            emit showPublishWindow();
            // TODO: add music
        }
        else
        {
            if(!_backgroundVideo.isNull())
                return;

            createVideoPlayer(true);
        }
        startPublishTimer();
    }
}

void EncounterScrollingTextEdit::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
    prepareImages();
    if((_videoPlayer) && (!_isDMPlayer))
    {
        createVideoPlayer(_isDMPlayer);
    }
}

void EncounterScrollingTextEdit::stopAnimation()
{
    publishClicked(false);
}

void EncounterScrollingTextEdit::rewind()
{
    _textPos.setY(_startPos);
}

void EncounterScrollingTextEdit::startPublishTimer()
{
    if(!_timerId)
    {
        _elapsed.start();
        _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
    }
}

void EncounterScrollingTextEdit::stopPublishTimer()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
}

void EncounterScrollingTextEdit::prepareImages()
{
    if(!_scrollingText)
        return;

    QSize rotatedSize = getRotatedTargetSize();

    if(!_backgroundImg.isNull())
        _prescaledImg = _backgroundImg.scaled(rotatedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    else
        _prescaledImg = _backgroundVideo.scaled(rotatedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(_rotation != 0)
    {
        _prescaledImg = _prescaledImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
    }

    _textImg = QImage();
}

void EncounterScrollingTextEdit::prepareTextImage()
{
    QSize scaledSize = _prescaledImg.size();
    if((_rotation == 90) || (_rotation == 270))
        scaledSize.transpose();

    QFont font(_scrollingText->getFontFamily(),
               _scrollingText->getFontSize(),
               _scrollingText->getFontBold() ? QFont::Bold : QFont::Normal,
               _scrollingText->getFontItalics());

    QRect targetRect(0, 0, scaledSize.width(), scaledSize.height());

    int textWidth = targetRect.width() * _scrollingText->getImageWidth() / 100;
    targetRect.setWidth(textWidth);
    _textPos.setX(static_cast<qreal>(scaledSize.width() - textWidth) / 2.0);

    QFontMetrics fontMetrics(font);
    QRect boundingRect = fontMetrics.boundingRect(targetRect, _scrollingText->getAlignment() | Qt::TextWordWrap, _scrollingText->getText());
    targetRect.setHeight(boundingRect.height());

    if((_rotation == 0) || (_rotation == 270))
        _startPos = static_cast<qreal>(scaledSize.height());
    else
        _startPos = static_cast<qreal>(boundingRect.height());

    if((!_animationRunning) || (_firstTime))
    {
        _textPos.setY(_startPos);
        _firstTime = false;
    }

    _textImg = QImage(targetRect.size(), QImage::Format_ARGB32_Premultiplied);
    _textImg.fill(Qt::transparent);
    {
        QPainter painter(&_textImg);
        painter.setFont(font);
        painter.setPen(_scrollingText->getFontColor());
        painter.drawText(targetRect, _scrollingText->getAlignment() | Qt::TextWordWrap, _scrollingText->getText());
    }
    if(_rotation != 0)
    {
        _textImg = _textImg.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
    }
}

void EncounterScrollingTextEdit::loadImage()
{
    if(!_scrollingText)
        return;

    _backgroundImg = QImage();
    _backgroundImgScaled = QImage();

    QString imageFileName = _scrollingText->getImageFile();
    if(!imageFileName.isEmpty())
    {
        if(_backgroundImg.load(imageFileName))
        {
            _backgroundImgScaled = _backgroundImg.scaled(ui->edtText->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        }
        else
        {
            createVideoPlayer(true);
            startPublishTimer();
        }
    }
}

void EncounterScrollingTextEdit::updateVideoBackground()
{
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    qDebug() << "[ScrollingText] Initializing battle map video background image";
    _backgroundVideo = _videoPlayer->getImage()->copy();
    _backgroundImgScaled = _backgroundVideo.scaled(ui->edtText->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    ui->edtText->update();

    _videoPlayer->stopThenDelete();
    _videoPlayer = nullptr;
}

void EncounterScrollingTextEdit::moveCursorToEnd()
{
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);
}

Qt::Alignment EncounterScrollingTextEdit::getAlignment()
{
    if(!_scrollingText)
        return Qt::AlignHCenter;

    switch(_scrollingText->getAlignment())
    {
        case Qt::AlignLeft:
        case Qt::AlignHCenter:
        case Qt::AlignRight:
            return _scrollingText->getAlignment();
        default:
            return Qt::AlignHCenter;
    }
}

QSize EncounterScrollingTextEdit::getRotatedTargetSize()
{
    QSize result;

    if(_rotation % 180 == 0)
    {
        result = _targetSize;
    }
    else
    {
        result = _targetSize.transposed();
    }

    return result;
}
