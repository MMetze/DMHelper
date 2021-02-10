#include "encountertextedit.h"
#include "encountertext.h"
#include "ui_encountertextedit.h"
#include <QKeyEvent>
#include <QTextCharFormat>
#include <QUrl>
#include <QPainter>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

EncounterTextEdit::EncounterTextEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::EncounterTextEdit),
    _keys(),
    _encounter(nullptr),
    _formatter(new TextEditFormatterFrame(this)),
    _backgroundImage(),
    _backgroundImageScaled(),
    _prescaledImage(),
    _textImage(),
    _targetSize(),
    _rotation(0)
{
    ui->setupUi(this);

    ui->textBrowser->viewport()->setCursor(Qt::IBeamCursor);

    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(storeEncounter()));
    connect(ui->textBrowser, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));

    connect(_formatter, SIGNAL(fontFamilyChanged(const QString&)), this, SIGNAL(fontFamilyChanged(const QString&)));
    connect(_formatter, SIGNAL(fontSizeChanged(int)), this, SIGNAL(fontSizeChanged(int)));
    connect(_formatter, SIGNAL(fontBoldChanged(bool)), this, SIGNAL(fontBoldChanged(bool)));
    connect(_formatter, SIGNAL(fontItalicsChanged(bool)), this, SIGNAL(fontItalicsChanged(bool)));
    connect(_formatter, SIGNAL(alignmentChanged(Qt::Alignment)), this, SIGNAL(alignmentChanged(Qt::Alignment)));
    connect(_formatter, SIGNAL(colorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));

    connect(_formatter, SIGNAL(fontFamilyChanged(const QString&)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontSizeChanged(int)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontBoldChanged(bool)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(fontItalicsChanged(bool)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(alignmentChanged(Qt::Alignment)), this, SLOT(takeFocus()));
    connect(_formatter, SIGNAL(colorChanged(QColor)), this, SLOT(takeFocus()));

    ui->textBrowser->installEventFilter(this);
    ui->textFormatter->hide();
    _formatter->setTextEdit(ui->textBrowser);
}

EncounterTextEdit::~EncounterTextEdit()
{
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

    emit checkableChanged(false);
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

void EncounterTextEdit::setHtml(const QString &text)
{
    ui->textBrowser->setHtml(text);
}

void EncounterTextEdit::setPlainText(const QString &text)
{
    ui->textBrowser->setPlainText(text);
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
    QString imageFileName = QFileDialog::getOpenFileName(this, QString("Select Image File"), QString(), QString("Image files (*.png *.jpg)"));

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

void EncounterTextEdit::setColor(QColor color)
{
    _formatter->setColor(color);
}

void EncounterTextEdit::setAlignment(Qt::Alignment alignment)
{
    _formatter->setAlignment(alignment);
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
    ui->textBrowser->setHtml(ui->textBrowser->toHtml());
}

void EncounterTextEdit::targetResized(const QSize& newSize)
{
    if(newSize != _targetSize)
        _targetSize = newSize;
}

void EncounterTextEdit::publishClicked(bool checked)
{
    Q_UNUSED(checked);

    if(!_encounter)
        return;

    emit showPublishWindow();

    prepareImages();
    emit publishImage(_textImage);
}

void EncounterTextEdit::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
}

void EncounterTextEdit::storeEncounter()
{
    if(_encounter)
        _encounter->setText(toHtml());
}

void EncounterTextEdit::readEncounter()
{
    disconnect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(storeEncounter()));
    if(_encounter)
        setHtml(_encounter->getText());
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

    if(_encounter->getImageFile().isEmpty())
        return;

    if(_backgroundImage.load(_encounter->getImageFile()))
        scaleBackgroundImage();
}

void EncounterTextEdit::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    scaleBackgroundImage();
}

void EncounterTextEdit::scaleBackgroundImage()
{
    if(_backgroundImage.isNull())
        return;

    _backgroundImageScaled = _backgroundImage.scaledToWidth(ui->textBrowser->width(), Qt::SmoothTransformation);
}

void EncounterTextEdit::prepareImages()
{
    if(!_encounter)
        return;

    if(_backgroundImage.isNull())
        return;

    QSize rotatedSize = getRotatedTargetSize();

    _prescaledImage = _backgroundImage.scaledToWidth(rotatedSize.width(), Qt::SmoothTransformation);
    if(_rotation != 0)
        _prescaledImage = _prescaledImage.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

    _textImage = QImage();
    prepareTextImage();
}

void EncounterTextEdit::prepareTextImage()
{
    //_textImage = QImage(getRotatedTargetSize(), QImage::Format_ARGB32);
    //ui->textBrowser->viewport()->render(&_textImage);
    QImage drawImage(ui->textBrowser->document()->size().toSize(), QImage::Format_ARGB32);
    drawImage.fill(QColor(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&drawImage);
        ui->textBrowser->document()->drawContents(&painter);
    painter.end();

    QSize rotatedSize = getRotatedTargetSize();

    drawImage = drawImage.scaledToWidth(rotatedSize.width(), Qt::SmoothTransformation);
    if(_rotation != 0)
        drawImage = drawImage.transformed(QTransform().rotate(_rotation), Qt::SmoothTransformation);

    _textImage = _prescaledImage;
    painter.begin(&_textImage);
        if((_rotation == 0) || (_rotation == 270))
            painter.drawImage(0, 0, drawImage);
        if(_rotation == 90)
            painter.drawImage(_prescaledImage.width() - drawImage.width(), 0, drawImage);
        if(_rotation == 180)
            painter.drawImage(0, _prescaledImage.height() - drawImage.height(), drawImage);
    painter.end();
}

QSize EncounterTextEdit::getRotatedTargetSize()
{
    if(_rotation % 180 == 0)
        return _targetSize;
    else
        return _targetSize.transposed();
}
