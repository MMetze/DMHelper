#include "encountertextedit.h"
#include "ui_encountertextedit.h"
#include "encountertext.h"
#include "publishgltextimagerenderer.h"
#include "publishgltextvideorenderer.h"
#include "dmconstants.h"
#include "campaign.h"
#include "texttranslatedialog.h"
#include "videoplayerglscreenshot.h"
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
    _isDMPlayer(false),
    _isPublishing(false),
    _targetSize(),
    _rotation(0),
    _textPos()
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
    delete _renderer;
    _renderer = nullptr;

    delete ui;
}

void EncounterTextEdit::activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer)
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

    if((currentRenderer) && (currentRenderer->getObject() == object))
        _renderer = dynamic_cast<PublishGLTextRenderer*>(currentRenderer);

    emit setPublishEnabled(true);
}

void EncounterTextEdit::deactivateObject()
{
    if(!_encounter)
    {
        qDebug() << "[EncounterTextEdit] WARNING: Invalid (nullptr) text encounter object deactivated!";
        return;
    }

    _renderer = nullptr;

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

    QFileInfo fileInfo(imageFileName);
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(nullptr,
                              QString("DMHelper Image Not Valid"),
                              QString("The selected image file isn't a file: ") + imageFileName);
        qDebug() << "[EncounterTextEdit] Image file not a file: " << imageFileName;
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

void EncounterTextEdit::rewind()
{
    if(_renderer)
        _renderer->rewind();
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
        _targetSize = newSize;
}

void EncounterTextEdit::publishClicked(bool checked)
{
    if((!_encounter) || ((_isPublishing == checked) && (_renderer) && (_renderer->getObject() == _encounter)))
        return;

    _isPublishing = checked;

    if(_isPublishing)
    {
        if(_renderer)
        {
            _renderer->play();
        }
        else
        {
            emit showPublishWindow();
            prepareImages();

            if(isVideo())
                _renderer = new PublishGLTextVideoRenderer(_encounter, _textImage);
            else
                _renderer = new PublishGLTextImageRenderer(_encounter, _prescaledImage, _textImage);

            emit registerRenderer(_renderer);
        }
    }
    else
    {
        if(_renderer)
            _renderer->stop();
    }
}

void EncounterTextEdit::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
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
        QFileInfo fileInfo(_encounter->getImageFile());
        if(fileInfo.isFile())
        {
            if(_backgroundImage.load(_encounter->getImageFile()))
                scaleBackgroundImage();
            else
                extractDMScreenshot();
        }
    }

    setPublishCheckable();
}

void EncounterTextEdit::handleScreenshotReady(const QImage& image)
{
    _backgroundImage = image;
    _backgroundImageScaled = QImage();
    scaleBackgroundImage();
    update();
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
}

void EncounterTextEdit::prepareImages()
{
    if(!_encounter)
        return;

    if(_backgroundImage.isNull())
    {
        _prescaledImage = QImage(_targetSize, QImage::Format_ARGB32_Premultiplied);
        _prescaledImage.fill(QColor(0, 0, 0, 0));
    }
    else
    {
        _prescaledImage = _backgroundImage;
    }

    _textImage = QImage();
    prepareTextImage();
}

void EncounterTextEdit::prepareTextImage()
{
    if((!_encounter) || (_prescaledImage.isNull()))
        return;

    _textImage = getDocumentTextImage();
    _textImage = _textImage.scaledToWidth(_prescaledImage.width(), Qt::SmoothTransformation);
}

QImage EncounterTextEdit::getDocumentTextImage()
{
    QImage result;

    QTextDocument* doc = ui->textBrowser->document();
    if(doc)
    {
        int oldTextWidth = doc->textWidth();
        int absoluteWidth = oldTextWidth * _encounter->getTextWidth() / 100;
        int targetMargin = (oldTextWidth - absoluteWidth) / 2;

        doc->setTextWidth(absoluteWidth);

        result = QImage(oldTextWidth, doc->size().height(), QImage::Format_ARGB32_Premultiplied);
        result.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&result);
            painter.translate(targetMargin, 0);
            doc->drawContents(&painter);
        painter.end();

        doc->setTextWidth(oldTextWidth);
    }

    return result;
}

void EncounterTextEdit::drawTextImage(QPaintDevice* target)
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

void EncounterTextEdit::extractDMScreenshot()
{
    if(!_encounter)
        return;

    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_encounter->getImageFile());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &EncounterTextEdit::handleScreenshotReady);
    screenshot->retrieveScreenshot();
}

bool EncounterTextEdit::isVideo() const
{
    if(!_encounter)
        return false;

    QFileInfo fileInfo(_encounter->getImageFile());
    if(!fileInfo.isFile())
        return false;

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
