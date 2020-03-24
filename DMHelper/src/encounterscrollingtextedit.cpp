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
    QFrame(parent),
    ui(new Ui::EncounterScrollingTextEdit),
    _scrollingText(nullptr),
    _backgroundWidth(0),
    _backgroundImg(),
    _prescaledImg(),
    _textImg(),
    _textPos(),
    _targetSize(),
    _elapsed(),
    _timerId(0),
    _videoPlayer(nullptr),
    _isDMPlayer(false),
    _backgroundVideo()
{
    ui->setupUi(this);

    connect(ui->btnBrowse,SIGNAL(clicked(bool)),this,SLOT(browseImageFile()));

    QFontDatabase fontDB;
    ui->cmbFont->addItems(fontDB.families());

    connect(ui->spinSpeed,SIGNAL(valueChanged(double)),this,SIGNAL(scrollSpeedChanged(double)));
    connect(ui->sliderWidth,SIGNAL(valueChanged(int)),this,SIGNAL(imageWidthChanged(int)));
    connect(ui->sliderWidth,SIGNAL(valueChanged(int)),this,SLOT(setTextWidth()));
    connect(ui->edtImageFile,SIGNAL(textChanged(QString)),this,SIGNAL(imgFileChanged(QString)));
    connect(ui->edtText,SIGNAL(textChanged()),this,SLOT(setPlainText()));
    connect(ui->cmbFont,SIGNAL(currentIndexChanged(QString)),this,SIGNAL(fontFamilyChanged(QString)));
    connect(ui->cmbFont,SIGNAL(currentIndexChanged(QString)),this,SLOT(setTextFont()));

    ui->edtSize->setValidator(new QIntValidator(1,999,this));
    connect(ui->edtSize,SIGNAL(textChanged(QString)),this,SLOT(setFontSize()));

    connect(ui->chkBold,SIGNAL(clicked(bool)),this,SIGNAL(fontBoldChanged(bool)));
    connect(ui->chkBold,SIGNAL(clicked(bool)),this,SLOT(setTextFont()));
    connect(ui->chkItalics,SIGNAL(clicked(bool)),this,SIGNAL(fontItalicsChanged(bool)));
    connect(ui->chkItalics,SIGNAL(clicked(bool)),this,SLOT(setTextFont()));

    ui->buttonGroup->setId(ui->btnLeftText,Qt::AlignLeft);
    ui->buttonGroup->setId(ui->btnCenterText,Qt::AlignHCenter);
    ui->buttonGroup->setId(ui->btnRightText,Qt::AlignRight);
    connect(ui->buttonGroup,SIGNAL(buttonClicked(int)),this,SLOT(setAlignment()));

    connect(ui->btnColor,SIGNAL(colorChanged(QColor)),this,SLOT(setColor()));
    ui->btnColor->setRotationVisible(false);

    connect(this,SIGNAL(imgFileChanged(const QString&)),this,SLOT(loadImage()));

    connect(ui->framePublish, SIGNAL(toggled(bool)), this, SLOT(runAnimation()));
    connect(ui->framePublish, SIGNAL(rotateCW()), this, SLOT(rotatePublish()));
    connect(ui->framePublish, SIGNAL(rotateCCW()), this, SLOT(rotatePublish()));

    ui->framePublish->setCheckable(true);

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

EncounterScrollingText* EncounterScrollingTextEdit::getScrollingText() const
{
    return _scrollingText;
}

void EncounterScrollingTextEdit::setScrollingText(EncounterScrollingText* scrollingText)
{
    qDebug() << "[Scrolling Text] Setting scrolling text to: " << scrollingText;

    if(_videoPlayer)
        cleanupPlayer();

    if(!scrollingText)
        return;

    _scrollingText = scrollingText;
    ui->spinSpeed->setValue(_scrollingText->getScrollSpeed()); // TODO: what if invalid?
    ui->sliderWidth->setValue(_scrollingText->getImageWidth());
    setTextWidth();
    ui->edtImageFile->setText(_scrollingText->getImgFile());
    ui->edtText->setPlainText(_scrollingText->getText());
    ui->cmbFont->setCurrentIndex(ui->cmbFont->findText(_scrollingText->getFontFamily()));
    ui->edtSize->setText(QString::number(_scrollingText->getFontSize()));
    ui->chkBold->setChecked(_scrollingText->getFontBold());
    ui->chkItalics->setChecked(_scrollingText->getFontItalics());
    setTextFont();
    ui->btnLeftText->setChecked(scrollingText->getAlignment() == Qt::AlignLeft);
    ui->btnCenterText->setChecked(scrollingText->getAlignment() == Qt::AlignHCenter);
    ui->btnRightText->setChecked(scrollingText->getAlignment() == Qt::AlignRight);

    setAlignment();

    ui->btnColor->setColor(_scrollingText->getFontColor());

    setColor();
}

void EncounterScrollingTextEdit::unsetScrollingText(EncounterScrollingText* scrollingText)
{
    Q_UNUSED(scrollingText);

    qDebug() << "[Scrolling Text] Unsetting scrolling text...";

    ui->framePublish->setChecked(false);
    cleanupPlayer();

    _scrollingText = nullptr;
}

void EncounterScrollingTextEdit::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
    {
        _targetSize = newSize;
        prepareImages();
        if((_videoPlayer) && (ui->framePublish->isChecked()))
        {
            _videoPlayer->targetResized(newSize);
        }
    }
}

void EncounterScrollingTextEdit::cancelPublish()
{
    ui->framePublish->cancelPublish();
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
        if((!ui->framePublish->isChecked()) ||
           (!_videoPlayer->isNewImage()) ||
           (!_videoPlayer->getImage()) || (_videoPlayer->getImage()->isNull()) ||
           (_videoPlayer->isError()) ||
           (!_videoPlayer->getMutex()))
            return;

        QMutexLocker locker(_videoPlayer->getMutex());
        if(ui->framePublish->getRotation() != 0)
        {
            targetImg = _videoPlayer->getImage()->transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
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
    if(ui->framePublish->getRotation() == 90)
    {
        drawPoint = QPointF(-drawPoint.y(), drawPoint.x());
    }
    else if(ui->framePublish->getRotation() == 180)
    {
        drawPoint.setY(-drawPoint.y());
    }
    else if(ui->framePublish->getRotation() == 270)
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
            if(!_backgroundImg.isNull())
            {
                QPainter paint(ui->edtText);
                paint.drawImage(0, 0, _backgroundImg);
            }
            else if(!_backgroundVideo.isNull())
            {
                QPainter paint(ui->edtText);
                paint.drawImage(0, 0, _backgroundVideo);
            }
        }
    }

    return QFrame::eventFilter(watched, event);
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
        _videoPlayer = new VideoPlayer(_scrollingText->getImgFile(), QSize(0, 0), true, false);
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
        _videoPlayer = new VideoPlayer(_scrollingText->getImgFile(), rotatedSize, true, false);
        //_videoPlayer->targetResized(rotatedSize);
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
    emit textChanged(ui->edtText->toPlainText());
}

void EncounterScrollingTextEdit::setFontSize()
{
    emit fontSizeChanged(ui->edtSize->text().toInt());
    setTextFont();
}

void EncounterScrollingTextEdit::setAlignment()
{
    Qt::AlignmentFlag newAlignment = getAlignment();

    ui->edtText->selectAll();
    ui->edtText->setAlignment(newAlignment);
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);

    emit alignmentChanged(newAlignment);
}

void EncounterScrollingTextEdit::setColor()
{
    QColor newColor = ui->btnColor->getColor();
    emit colorChanged(newColor);

    // Set the text color for all text then move the cursor to the end
    ui->edtText->selectAll();
    ui->edtText->setTextColor(newColor);
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);
}

void EncounterScrollingTextEdit::setTextFont()
{
    ui->edtText->selectAll();
    ui->edtText->setFontFamily(ui->cmbFont->currentText());
    ui->edtText->setFontItalic(ui->chkItalics->isChecked());
    ui->edtText->setFontWeight(ui->chkBold->isChecked() ? QFont::Bold : QFont::Normal);
    ui->edtText->setFontPointSize(ui->edtSize->text().toInt());

    // Move the cursor to the end of the text
    QTextCursor cursor(ui->edtText->textCursor());
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    ui->edtText->setTextCursor(cursor);
}

void EncounterScrollingTextEdit::setTextWidth()
{
    ui->edtText->setTextWidth(ui->sliderWidth->value());
}

void EncounterScrollingTextEdit::browseImageFile()
{
    QString imageFileName = QFileDialog::getOpenFileName(this,QString("Select Image File"));

    if((imageFileName.isEmpty()) || (!QFile::exists(imageFileName)))
    {
        qDebug() << "[ScrollingText] Invalid image file selected for animation";
        return;
    }

    ui->edtImageFile->setText(imageFileName);
    emit imgFileChanged(imageFileName);
}

void EncounterScrollingTextEdit::runAnimation()
{
    if(!_backgroundImg.isNull())
    {
        if(ui->framePublish->isChecked())
        {
            prepareImages();

            emit animationStarted(ui->framePublish->getColor());
            emit showPublishWindow();
            // TODO: add music

            startPublishTimer();
        }
        else
        {
            stopPublishTimer();
        }
    }
    else if(!_scrollingText->getImgFile().isEmpty())
    {
        stopPublishTimer();
        if(ui->framePublish->isChecked())
        {
            prepareImages();
            createVideoPlayer(false);

            emit animationStarted(ui->framePublish->getColor());
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

void EncounterScrollingTextEdit::rotatePublish()
{
    prepareImages();
    if((_videoPlayer) && (!_isDMPlayer))
    {
        createVideoPlayer(_isDMPlayer);
    }
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

    if(ui->framePublish->getRotation() != 0)
    {
        _prescaledImg = _prescaledImg.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
    }

    _textImg = QImage();
}

void EncounterScrollingTextEdit::prepareTextImage()
{
    QSize scaledSize = _prescaledImg.size();
    if((ui->framePublish->getRotation() == 90) || (ui->framePublish->getRotation() == 270))
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
    QRect boundingRect = fontMetrics.boundingRect(targetRect, static_cast<Qt::AlignmentFlag>(_scrollingText->getAlignment()) | Qt::TextWordWrap, _scrollingText->getText());
    targetRect.setHeight(boundingRect.height());

    if((ui->framePublish->getRotation() == 0) || (ui->framePublish->getRotation() == 270))
        _textPos.setY(static_cast<qreal>(scaledSize.height()));
    else
        _textPos.setY(static_cast<qreal>(boundingRect.height()));

    _textImg = QImage(targetRect.size(), QImage::Format_ARGB32_Premultiplied);
    _textImg.fill(Qt::transparent);
    {
        QPainter painter(&_textImg);
        painter.setFont(font);
        painter.setPen(_scrollingText->getFontColor());
        painter.drawText(targetRect, static_cast<Qt::AlignmentFlag>(_scrollingText->getAlignment()) | Qt::TextWordWrap, _scrollingText->getText());
    }
    if(ui->framePublish->getRotation() != 0)
    {
        _textImg = _textImg.transformed(QTransform().rotate(ui->framePublish->getRotation()), Qt::SmoothTransformation);
    }
}

void EncounterScrollingTextEdit::loadImage()
{
    _backgroundImg = QImage();

    QString imageFileName = ui->edtImageFile->text();
    if(!imageFileName.isEmpty())
    {
        if(!_backgroundImg.load(ui->edtImageFile->text()))
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
    ui->edtText->update();

    _videoPlayer->stopThenDelete();
    _videoPlayer = nullptr;
}

Qt::AlignmentFlag EncounterScrollingTextEdit::getAlignment()
{
    switch(ui->buttonGroup->checkedId())
    {
    case Qt::AlignLeft:
    case Qt::AlignHCenter:
    case Qt::AlignRight:
        return static_cast<Qt::AlignmentFlag>(ui->buttonGroup->checkedId());
    default:
        return Qt::AlignHCenter;
    }
}

QSize EncounterScrollingTextEdit::getRotatedTargetSize()
{
    QSize result;

    if(ui->framePublish->getRotation() % 180 == 0)
    {
        result = _targetSize;
    }
    else
    {
        result = _targetSize.transposed(); //_targetSize.transposed().scaled(_targetSize, Qt::KeepAspectRatio).transposed();
    }

    return result;
}
