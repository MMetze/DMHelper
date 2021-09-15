#include "encountertextedit.h"
#include "encountertext.h"
#include "publishgltextrenderer.h"
#include "dmconstants.h"
#include "campaign.h"
#include "texttranslatedialog.h"
#include "ui_encountertextedit.h"
#include <QKeyEvent>
#include <QTextCharFormat>
#include <QUrl>
#include <QPainter>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>

EncounterTextEdit::EncounterTextEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::EncounterTextEdit),
    _keys(),
    _encounter(nullptr),
    _renderer(nullptr),
    _formatter(new TextEditFormatterFrame(this)),
    _backgroundImage(),
    _backgroundImageScaled(),
    _prescaledImage(),
    _textImage(),
    _videoPlayer(nullptr),
    _isDMPlayer(false),
    _backgroundVideo(),
    _targetSize(),
    _rotation(0),
    _animationRunning(false),
    _textPos(),
    _elapsed(),
    _timerId(0)
{
    ui->setupUi(this);

    ui->textBrowser->viewport()->setCursor(Qt::IBeamCursor);

    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(storeEncounter()));
    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(updateAnchors()));
    connect(ui->textBrowser, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));

    connect(_formatter, SIGNAL(fontFamilyChanged(const QString&)), this, SIGNAL(fontFamilyChanged(const QString&)));
    connect(_formatter, SIGNAL(fontSizeChanged(int)), this, SIGNAL(fontSizeChanged(int)));
    connect(_formatter, SIGNAL(fontBoldChanged(bool)), this, SIGNAL(fontBoldChanged(bool)));
    connect(_formatter, SIGNAL(fontItalicsChanged(bool)), this, SIGNAL(fontItalicsChanged(bool)));
    connect(_formatter, SIGNAL(fontUnderlineChanged(bool)), this, SIGNAL(fontUnderlineChanged(bool)));
    connect(_formatter, SIGNAL(alignmentChanged(Qt::Alignment)), this, SIGNAL(alignmentChanged(Qt::Alignment)));
    connect(_formatter, SIGNAL(colorChanged(const QColor&)), this, SIGNAL(colorChanged(const QColor&)));

    connect(_formatter, SIGNAL(fontFamilyChanged(const QString&)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontSizeChanged(int)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontBoldChanged(bool)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontItalicsChanged(bool)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontUnderlineChanged(bool)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(colorChanged(const QColor&)), this, SLOT(takeFocus()));

    ui->textBrowser->installEventFilter(this);
    ui->textFormatter->hide();
    _formatter->setTextEdit(ui->textBrowser);
}

EncounterTextEdit::~EncounterTextEdit()
{
    stopPublishTimer();

    delete _renderer;
    _renderer = nullptr;

    if(_videoPlayer)
    {
        VideoPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        delete deletePlayer;
    }

    delete ui;
}

void EncounterTextEdit::activateObject(CampaignObjectBase* object)
{
    EncounterText* encounter = dynamic_cast<EncounterText*>(object);
    if(!encounter)
        return;

    if(_encounter != nullptr)
    {
        qDebug() << "[EncounterTextEdit] ERROR: New text encounter object activated without deactivating the existing text encounter object first!";
        deactivateObject();
    }

    setEncounter(encounter);

    emit setPublishEnabled(true);
}

void EncounterTextEdit::deactivateObject()
{
    if(!_encounter)
    {
        qDebug() << "[EncounterTextEdit] WARNING: Invalid (nullptr) text encounter object deactivated!";
        return;
    }

    storeEncounter();
    unsetEncounter(_encounter);
}

void EncounterTextEdit::setKeys(const QList<QString>& keys)
{
    _keys = keys;
}

QList<QString> EncounterTextEdit::keys()
{
    return _keys;
}

EncounterText* EncounterTextEdit::getEncounter() const
{
    return _encounter;
}

void EncounterTextEdit::setEncounter(EncounterText* encounter)
{
    if(_encounter == encounter)
        return;

    if(!encounter)
    {
        unsetEncounter(_encounter);
        return;
    }

    storeEncounter();

    _encounter = encounter;

    readEncounter();
    connect(_encounter, SIGNAL(imageFileChanged(const QString&)), this, SIGNAL(imageFileChanged(const QString&)));
    connect(_encounter, SIGNAL(imageFileChanged(const QString&)), this, SLOT(loadImage()));
    connect(_encounter, SIGNAL(textWidthChanged(int)), this, SIGNAL(textWidthChanged(int)));
    connect(_encounter, &EncounterText::textWidthChanged, ui->textBrowser, &TextBrowserMargins::setTextWidth);
    connect(_encounter, &EncounterText::scrollSpeedChanged, this, &EncounterTextEdit::scrollSpeedChanged);
    connect(_encounter, &EncounterText::animatedChanged, this, &EncounterTextEdit::animatedChanged);
    connect(_encounter, &EncounterText::translatedChanged, this, &EncounterTextEdit::translatedChanged);
}

void EncounterTextEdit::unsetEncounter(EncounterText* encounter)
{
    if(encounter != _encounter)
        qDebug() << "[EncounterTextEdit] WARNING: unsetting text with a DIFFERENT encounter than currently set! Current: " << QString(_encounter ? _encounter->getID().toString() : "nullptr") << ", Unset: " << QString(encounter ? encounter->getID().toString() : "nullptr");

    if(_encounter)
    {        
        if(isAnimated())
            stopAnimation();

        cleanupPlayer();

        disconnect(_encounter, nullptr, this, nullptr);
        _encounter = nullptr;
    }

    _backgroundImage = QImage();
    _backgroundImageScaled = QImage();
    ui->textBrowser->clear();
}

QString EncounterTextEdit::toHtml() const
{
    return ui->textBrowser->toHtml();
}

QString EncounterTextEdit::toPlainText() const
{
    return ui->textBrowser->toPlainText();
}

bool EncounterTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if(watched != ui->textBrowser)
        return false;

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(!keyEvent)
            return false;

        switch(keyEvent->key())
        {
            case Qt::Key_B:
                if( (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
                {
                    QTextCharFormat format = ui->textBrowser->currentCharFormat();
                    format.setFontWeight(format.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                    ui->textBrowser->setCurrentCharFormat(format);
                    return true;
                }
                break;

            case Qt::Key_I:
                if( (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
                {
                    QTextCharFormat format = ui->textBrowser->currentCharFormat();
                    format.setFontItalic(!format.fontItalic());
                    ui->textBrowser->setCurrentCharFormat(format);
                    return true;
                }
                break;

            case Qt::Key_U:
                if( (keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
                {
                    QTextCharFormat format = ui->textBrowser->currentCharFormat();
                    format.setFontUnderline(!format.fontUnderline());
                    ui->textBrowser->setCurrentCharFormat(format);
                    return true;
                }
                break;
        }
    }
    else if(event->type() == QEvent::Paint)
    {
        if(!_backgroundImageScaled.isNull())
        {
            QPainter paint(ui->textBrowser);
            paint.drawImage(0, 0, _backgroundImageScaled);
        }
    }

    return false;
}

void EncounterTextEdit::clear()
{
    _keys.clear();
    ui->textBrowser->setPlainText(QString(""));
}

void EncounterTextEdit::setHtml()
{
    if(!_encounter)
    {
        ui->textBrowser->clear();
        return;
    }

    if(_encounter->getTranslated())
        ui->textBrowser->setHtml(_encounter->getTranslatedText());
    else
        ui->textBrowser->setHtml(_encounter->getText());

    updateAnchors();
}

void EncounterTextEdit::setBackgroundImage(bool on)
{
    if(!_encounter)
        return;

    if(on)
    {
        browseImageFile();
    }
    else
    {
        setImageFile(QString());
        ui->textBrowser->update();
    }
}

void EncounterTextEdit::setImageFile(const QString& imageFile)
{
    if(_encounter)
        _encounter->setImageFile(imageFile);
}

void EncounterTextEdit::browseImageFile()
{
    QString imageFileName = QFileDialog::getOpenFileName(this, QString("Select Image File"));

    if(imageFileName.isEmpty())
    {
        qDebug() << "[EncounterTextEdit] Select Image File was cancelled";
        return;
    }

    if(!QFile::exists(imageFileName))
    {
        QMessageBox::critical(this, QString("Invalid Image File"), QString("The selected image could not be found!"));
        qDebug() << "[EncounterTextEdit] Invalid image file selected for text background";
        return;
    }

    setImageFile(imageFileName);
}

void EncounterTextEdit::setFont(const QString& fontFamily)
{
    _formatter->setFont(fontFamily);
}

void EncounterTextEdit::setFontSize(int fontSize)
{
    _formatter->setFontSize(fontSize);
}

void EncounterTextEdit::setBold(bool bold)
{
    _formatter->setBold(bold);
}

void EncounterTextEdit::setItalics(bool italics)
{
    _formatter->setItalics(italics);
}

void EncounterTextEdit::setUnderline(bool underline)
{
    _formatter->setUnterline(underline);
}

void EncounterTextEdit::setColor(const QColor& color)
{
    _formatter->setColor(color);
}

void EncounterTextEdit::setAlignment(Qt::Alignment alignment)
{
    _formatter->setAlignment(alignment);
}

void EncounterTextEdit::setPasteRich(bool pasteRich)
{
    ui->textBrowser->setAcceptRichText(pasteRich);
}

void EncounterTextEdit::hyperlinkClicked()
{
    QTextCursor cursor = ui->textBrowser->textCursor();
    QTextCharFormat format = cursor.charFormat();

    bool result = false;
    QString newHRef = QInputDialog::getText(nullptr,
                                            QString("Edit Hyperlink"),
                                            QString("Enter the Hyperlink for the selected text: "),
                                            QLineEdit::Normal,
                                            format.anchorHref(),
                                            &result);
    if(!result)
        return;

    if(!newHRef.isEmpty())
    {
        if(!(QUrl(newHRef).isValid()))
        {
            qDebug() << "[EncounterTextEdit] Invalid URL detected: " << newHRef;
            QMessageBox::critical(nullptr,
                                  QString("Hyperlink Error"),
                                  QString("The provided hyperlink is not valid: ") + newHRef);
        }
    }

    format.setAnchor(!newHRef.isEmpty());
    format.setAnchorHref(newHRef);
    cursor.mergeCharFormat(format);
}

void EncounterTextEdit::setTextWidth(int textWidth)
{
    if(_encounter)
        _encounter->setTextWidth(textWidth);
}

void EncounterTextEdit::setAnimated(bool animated)
{
    if(!_encounter)
        return;

    _encounter->setAnimated(animated);
    setPublishCheckable();
}

void EncounterTextEdit::setScrollSpeed(int scrollSpeed)
{
    if(_encounter)
        _encounter->setScrollSpeed(scrollSpeed);
}

void EncounterTextEdit::stopAnimation()
{
    publishClicked(false);
}

void EncounterTextEdit::rewind()
{
    qreal yPos = 0.0;
    if(_encounter->getAnimated())
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

void EncounterTextEdit::setTranslated(bool translated)
{
    if((!_encounter) || (_encounter->getTranslated() == translated))
        return;

    if(translated)
    {
        TextTranslateDialog dlg(_encounter->getText(), _encounter->getTranslatedText(), _backgroundImage);
        dlg.resize(width() / 2, height() / 2);
        if(dlg.exec() == QDialog::Accepted)
        {
            qDebug() << "[EncounterTextEdit] Translation result accepted: " << translated;
            _encounter->setTranslated(true);
            QString translatedText;
            dlg.getTranslatedText(translatedText);
            _encounter->setTranslatedText(translatedText);
        }
        else
        {
            qDebug() << "[EncounterTextEdit] Translation result rejected: " << translated;
            //_encounter->setTranslated(false);
            emit translatedChanged(false);
        }
    }
    else
    {
        qDebug() << "[EncounterTextEdit] Translation deactivated" ;
        _encounter->setTranslated(false);
    }

    setHtml();
}

void EncounterTextEdit::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
    {
        int oldHeight = _targetSize.height();
        _targetSize = newSize;
        if((_encounter) && (isAnimated()))
        {
            prepareImages();

            if((_encounter->getAnimated()) && (oldHeight > 0) && (newSize.height() > 0))
                _textPos.setY(_textPos.y() * newSize.height() / oldHeight);
        }

        if(_videoPlayer)
            _videoPlayer->targetResized(newSize);
    }
}

void EncounterTextEdit::publishClicked(bool checked)
{
    Q_UNUSED(checked);

    if(!_encounter)
        return;

    qDebug() << "[EncounterTextEdit] Publish clicked. Checked: " << checked << ", animated: " << _encounter->getAnimated() << ", running: " << _animationRunning << ", background image: " << _backgroundImage.size() << ", file: " << _encounter->getImageFile();

    if(_encounter->getAnimated())
    {
        if(_animationRunning == checked)
            return;

        if(checked)
        {
            /*
            emit showPublishWindow();
            prepareImages();

            if(isVideo())
                createVideoPlayer(false);

            emit animationStarted();
            startPublishTimer();
            */
            emit showPublishWindow();
            prepareImages();
            if(!_renderer)
                _renderer = new PublishGLTextRenderer(_prescaledImage, _textImage);
            emit registerRenderer(_renderer);

        }
        else
        {
            stopPublishTimer();
            if(isVideo())
                createVideoPlayer(true);
        }

        _animationRunning = checked;
    }
    else
    {
        if((isVideo()) && (_animationRunning == checked))
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
        else if(_encounter)
        {
            stopPublishTimer();
            if(checked)
            {
                createVideoPlayer(false);
                emit animationStarted();
            }
            else
            {
                if(!_backgroundVideo.isNull())
                {
                    _animationRunning = checked;
                    return;
                }

                createVideoPlayer(true);
            }
            startPublishTimer();

            _animationRunning = checked;
        }
    }
}

void EncounterTextEdit::setRotation(int rotation)
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

void EncounterTextEdit::updateAnchors()
{
    if(!_encounter)
        return;

    qDebug() << "[EncounterTextEdit] Checking anchors...";
    disconnect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(updateAnchors()));

    QTextCursor originalCursor = ui->textBrowser->textCursor();

    QTextCursor startPosCursor = originalCursor;
    startPosCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    ui->textBrowser->setTextCursor(startPosCursor);

    QRegularExpression regex("\\[\\[(.*?)\\]\\]");
    while(ui->textBrowser->find(regex))
    {
        QTextCursor cursor = ui->textBrowser->textCursor();
        int startPos = cursor.selectionStart();
        int endPos = cursor.selectionEnd();
        cursor.setPosition(startPos + 2);
        cursor.setPosition(endPos - 2, QTextCursor::KeepAnchor);
        if(!cursor.selectedText().isEmpty())
        {
            Campaign* campaign = dynamic_cast<Campaign*>(_encounter->getParentByType(DMHelper::CampaignType_Campaign));
            if(campaign)
            {
                QTextCharFormat format = cursor.charFormat();
                if(campaign->searchDirectChildrenByName(cursor.selectedText()))
                {
                    qDebug() << "[EncounterTextEdit] Setting cursor format for text: " << cursor.selectedText();
                    format.setAnchor(true);
                    format.setAnchorHref(QString("DMHelper@") + cursor.selectedText());
                    format.setFontItalic(true);
                    cursor.mergeCharFormat(format);
                }
                else if(format.isAnchor())
                {
                    qDebug() << "[EncounterTextEdit] Removing cursor format for text: " << cursor.selectedText();
                    format.setAnchor(false);
                    format.setAnchorHref(QString(""));
                    format.setFontItalic(false);
                    cursor.mergeCharFormat(format);
                }
            }
        }
    }

    ui->textBrowser->setTextCursor(originalCursor);
    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(updateAnchors()));
}

void EncounterTextEdit::storeEncounter()
{
    if(!_encounter)
        return;

    if(_encounter->getTranslated())
        _encounter->setTranslatedText(toHtml());
    else
        _encounter->setText(toHtml());
}

void EncounterTextEdit::readEncounter()
{
    disconnect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(storeEncounter()));
    if(_encounter)
    {
        //setHtml(_encounter->getText());

        emit imageFileChanged(_encounter->getImageFile());
        emit textWidthChanged(_encounter->getTextWidth());
        emit animatedChanged(_encounter->getAnimated());
        emit scrollSpeedChanged(_encounter->getScrollSpeed());
        emit translatedChanged(_encounter->getTranslated());

        ui->textBrowser->setTextWidth(_encounter->getTextWidth());
        loadImage();

        setAnimated(_encounter->getAnimated());
        setTranslated(_encounter->getTranslated());
        setHtml();
    }
    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(storeEncounter()));
}

void EncounterTextEdit::takeFocus()
{
    update();
    ui->textBrowser->update();
    ui->textBrowser->setFocus();
}

void EncounterTextEdit::loadImage()
{
    if(!_encounter)
        return;

    _backgroundImage = QImage();
    _backgroundImageScaled = QImage();

    if(!_encounter->getImageFile().isEmpty())
    {
        if(_backgroundImage.load(_encounter->getImageFile()))
            scaleBackgroundImage();
        else
            createVideoPlayer(true);
    }

    setPublishCheckable();
}

void EncounterTextEdit::updateVideoBackground()
{
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    qDebug() << "[TextEdit] Initializing battle map video background image";
    _backgroundVideo = _videoPlayer->getImage()->copy();
    _backgroundImageScaled = _backgroundVideo.scaled(ui->textBrowser->size(), Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    ui->textBrowser->update();

    _videoPlayer->stopThenDelete();
    _videoPlayer = nullptr;
}

void EncounterTextEdit::startPublishTimer()
{
    if(!_timerId)
    {
        rewind();
        if(_encounter->getAnimated())
            _elapsed.start();
        _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
    }
}

void EncounterTextEdit::stopPublishTimer()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
}

void EncounterTextEdit::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(!_encounter)
        return;

    qreal elapsedtime = 0.0;
    if(_encounter->getAnimated())
    {
        elapsedtime = _elapsed.restart();
        _textPos.ry() -= _encounter->getScrollSpeed() * (_prescaledImage.height() / 250) * (elapsedtime / 1000.0);
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

    if((_encounter->getAnimated()) && (_textPos.y() < -_textImage.height()))
        emit animationStopped();
}

void EncounterTextEdit::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    scaleBackgroundImage();
}

void EncounterTextEdit::scaleBackgroundImage()
{
    if(!_backgroundImage.isNull())
        _backgroundImageScaled = _backgroundImage.scaledToWidth(ui->textBrowser->width(), Qt::FastTransformation);
    else if(!_backgroundVideo.isNull())
        _backgroundImageScaled = _backgroundVideo.scaledToWidth(ui->textBrowser->width(), Qt::FastTransformation);
}

void EncounterTextEdit::prepareImages()
{
    if(!_encounter)
        return;

    if(_backgroundImage.isNull())
    {
        //_prescaledImage = QImage(_targetSize, QImage::Format_ARGB32);
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

void EncounterTextEdit::prepareTextImage()
{
    if((!_encounter) || (_prescaledImage.isNull()))
        return;

    QTextDocument* doc = ui->textBrowser->document();
    if(!doc)
        return;

    int oldTextWidth = doc->textWidth();
    int absoluteWidth = oldTextWidth * _encounter->getTextWidth() / 100;
    int targetMargin = (oldTextWidth - absoluteWidth) / 2;

    doc->setTextWidth(absoluteWidth);

    _textImage = QImage(oldTextWidth, doc->size().height(), QImage::Format_ARGB32_Premultiplied);
    //_textImage = QImage(absoluteWidth, doc->size().height(), QImage::Format_ARGB32_Premultiplied);
    _textImage.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&_textImage);
        painter.translate(targetMargin, 0);
        doc->drawContents(&painter);
    painter.end();

    doc->setTextWidth(oldTextWidth);

    QSize rotatedSize = getRotatedTargetSize();
    //int rotatedWidth = rotatedSize.width() * _encounter->getTextWidth() / 100;

    _textImage = _textImage.scaledToWidth(rotatedSize.width(), Qt::SmoothTransformation);
    //_textImage = _textImage.scaledToWidth(rotatedWidth, Qt::SmoothTransformation);
    if(_rotation != 0)
        _textImage = _textImage.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);
}

void EncounterTextEdit::drawTextImage(QPaintDevice* target)
{
    if(!target)
        return;

    QPainter painter(target);
    QPointF drawPoint(0.0, 0.0);

    if(_rotation == 0)
        drawPoint = QPointF(0.0, _textPos.y()); // painter.drawImage(0, _textPos.y(), _textImage);
    else if(_rotation == 90)
        drawPoint = QPointF(_prescaledImage.width() - _textImage.width() - _textPos.y(), 0.0); // painter.drawImage(_prescaledImage.width() - _textImage.width() - _textPos.y(), 0, _textImage);
    else if(_rotation == 180)
        drawPoint = QPointF(0.0, _prescaledImage.height() - _textImage.height() - _textPos.y()); // painter.drawImage(0, _prescaledImage.height() - _textImage.height() - _textPos.y(), _textImage);
    else if(_rotation == 270)
        drawPoint = QPointF(_textPos.y(), 0.0); // painter.drawImage(_textPos.y(), 0, _textImage);

    painter.drawImage(drawPoint, _textImage, _textImage.rect());
}

void EncounterTextEdit::createVideoPlayer(bool dmPlayer)
{
    if(!_encounter)
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
        _videoPlayer = new VideoPlayer(_encounter->getImageFile(), QSize(0, 0), true, false);
        if(_videoPlayer->isNewImage())
            updateVideoBackground();
        else
            connect(_videoPlayer, &VideoPlayer::screenShotAvailable, this, &EncounterTextEdit::updateVideoBackground);
    }
    else
    {
        qDebug() << "[ScrollingText] Publish FoW Player animation started";

        // TODO: consider audio in the scrolling text
        QSize rotatedSize = getRotatedTargetSize();
        _videoPlayer = new VideoPlayer(_encounter->getImageFile(), rotatedSize, true, false);
    }
}

void EncounterTextEdit::cleanupPlayer()
{
    stopPublishTimer();
    if(_videoPlayer)
    {
        _videoPlayer->stopThenDelete();
        _videoPlayer = nullptr;
    }
}

bool EncounterTextEdit::isVideo() const
{
    if(!_encounter)
        return false;
    else
        return((_backgroundImage.isNull()) && (!_encounter->getImageFile().isEmpty()));
}

bool EncounterTextEdit::isAnimated() const
{
    if(!_encounter)
        return false;
    else
        return((isVideo()) || (_encounter->getAnimated()));
}

void EncounterTextEdit::setPublishCheckable()
{
    emit checkableChanged(isAnimated());
}

QSize EncounterTextEdit::getRotatedTargetSize()
{
    if(_rotation % 180 == 0)
        return _targetSize;
    else
        return _targetSize.transposed();
}
