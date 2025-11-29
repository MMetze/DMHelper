#include "encountertextedit.h"
#include "ui_encountertextedit.h"
#include "encountertext.h"
#include "encountertextlinked.h"
#include "publishgltextrenderer.h"
#include "dmconstants.h"
#include "campaign.h"
#include "texttranslatedialog.h"
#include "layerseditdialog.h"
#include <QKeyEvent>
#include <QTextCharFormat>
#include <QUrl>
#include <QPainter>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollBar>
#include <QDebug>

const int ENCOUNTERTEXTEDIT_STORE_INTERVAL = 3000;
const int ENCOUNTERTEXTEDIT_ANCHOR_UPDATE_INTERVAL = 500;

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
    _isCodeView(false),
    _targetSize(),
    _rotation(0),
    _textPos(),
    _encounterChangedTimer(0),
    _updateAnchorTimer(0)
{
    ui->setupUi(this);

    ui->textBrowser->viewport()->setCursor(Qt::IBeamCursor);

    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(triggerEncounterChanged()));
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
    {
        _renderer = dynamic_cast<PublishGLTextRenderer*>(currentRenderer);
        if(_renderer)
            _renderer->setRotation(_rotation);
    }

    emit setPublishEnabled(true, true);
    emit setLayers(_encounter->getLayerScene().getLayers(), _encounter->getLayerScene().getSelectedLayerIndex());
}

void EncounterTextEdit::deactivateObject()
{
    if(!_encounter)
    {
        qDebug() << "[EncounterTextEdit] WARNING: Invalid (nullptr) text encounter object deactivated!";
        return;
    }

    cancelTimers();

    _renderer = nullptr;

    storeEncounter();
    unsetEncounter(_encounter);

    emit setLayers(QList<Layer*>(), 0);
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

    _encounter->initialize();

    readEncounter();
    connect(_encounter, SIGNAL(textWidthChanged(int)), this, SIGNAL(textWidthChanged(int)));
    connect(_encounter, &EncounterText::textChanged, this, &EncounterTextEdit::readEncounter);
    connect(_encounter, &EncounterText::textWidthChanged, ui->textBrowser, &TextBrowserMargins::setTextWidth);
    connect(_encounter, &EncounterText::scrollSpeedChanged, this, &EncounterTextEdit::scrollSpeedChanged);
    connect(_encounter, &EncounterText::animatedChanged, this, &EncounterTextEdit::animatedChanged);
    connect(_encounter, &EncounterText::translatedChanged, this, &EncounterTextEdit::translatedChanged);
    connect(&_encounter->getLayerScene(), &LayerScene::sceneChanged, this, &EncounterTextEdit::handleLayersChanged);
    connect(&_encounter->getLayerScene(), &LayerScene::layerAdded, this, &EncounterTextEdit::refreshImage);
    connect(&_encounter->getLayerScene(), &LayerScene::layerRemoved, this, &EncounterTextEdit::refreshImage);
    connect(&_encounter->getLayerScene(), &LayerScene::sceneChanged, this, &EncounterTextEdit::refreshImage);
    connect(&_encounter->getLayerScene(), &LayerScene::dirty, this, &EncounterTextEdit::refreshImage);

    if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
    {
        EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
        if(linkedText)
        {
            connect(_encounter, &EncounterText::textChanged, this, &EncounterTextEdit::updateEncounter);
            linkedText->setWatcher(true);
        }
    }
}

void EncounterTextEdit::unsetEncounter(EncounterText* encounter)
{
    if(encounter != _encounter)
        qDebug() << "[EncounterTextEdit] WARNING: unsetting text with a DIFFERENT encounter than currently set! Current: " << QString(_encounter ? _encounter->getID().toString() : "nullptr") << ", Unset: " << QString(encounter ? encounter->getID().toString() : "nullptr");

    cancelTimers();

    if(_encounter)
    {
        if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
        {
            EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
            if(linkedText)
                linkedText->setWatcher(false);
        }

        _encounter->uninitialize();

        disconnect(&_encounter->getLayerScene(), &LayerScene::sceneChanged, this, &EncounterTextEdit::handleLayersChanged);
        disconnect(&_encounter->getLayerScene(), &LayerScene::layerAdded, this, &EncounterTextEdit::refreshImage);
        disconnect(&_encounter->getLayerScene(), &LayerScene::layerRemoved, this, &EncounterTextEdit::refreshImage);
        disconnect(&_encounter->getLayerScene(), &LayerScene::sceneChanged, this, &EncounterTextEdit::refreshImage);
        disconnect(&_encounter->getLayerScene(), &LayerScene::dirty, this, &EncounterTextEdit::refreshImage);
        disconnect(_encounter, nullptr, this, nullptr);
        _encounter = nullptr;
    }

    _backgroundImage = QImage();
    _backgroundImageScaled = QImage();
    ui->textBrowser->clear();
}

QString EncounterTextEdit::toHtml() const
{
    if(_encounter->getObjectType() == DMHelper::CampaignType_Text)
    {
        return ui->textBrowser->toHtml();
    }
    else if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
    {
        EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
        if(linkedText)
        {
            if((_isCodeView) || (linkedText->getFileType() == DMHelper::FileType_Text))
                return ui->textBrowser->toPlainText();
            else if(linkedText->getFileType() == DMHelper::FileType_HTML)
                ui->textBrowser->toHtml();
            else if(linkedText->getFileType() == DMHelper::FileType_Markdown)
                ui->textBrowser->toMarkdown();
        }
    }

    return QString();
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

        if((keyEvent->key() == Qt::Key_BracketLeft) || (keyEvent->key() == Qt::Key_BracketRight))
        {
            triggerUpdateAnchor(); // Don't swallow the event, just trigger the update
        }
        else if((_formatter) && ((keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
        {
            if(keyEvent->key() == Qt::Key_B)
            {
                _formatter->setBold(ui->textBrowser->fontWeight() == QFont::Normal);
                return true;
            }
            else if(keyEvent->key() == Qt::Key_I)
            {
                _formatter->setItalics(!ui->textBrowser->fontItalic());
                return true;
            }
            else if(keyEvent->key() == Qt::Key_U)
            {
                _formatter->setUnterline(!ui->textBrowser->fontUnderline());
                return true;
            }
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

    return CampaignObjectFrame::eventFilter(watched, event);
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

    if(_encounter->getObjectType() == DMHelper::CampaignType_Text)
    {
        if(_encounter->getTranslated())
            ui->textBrowser->setHtml(_encounter->getTranslatedText());
        else
            ui->textBrowser->setHtml(_encounter->getText());
    }
    else if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
    {
        EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
        if(linkedText)
        {
            if((_isCodeView) || (linkedText->getFileType() == DMHelper::FileType_Text))
                ui->textBrowser->setPlainText(_encounter->getText());
            else if(linkedText->getFileType() == DMHelper::FileType_HTML)
                ui->textBrowser->setHtml(_encounter->getText());
            else if(linkedText->getFileType() == DMHelper::FileType_Markdown)
                ui->textBrowser->setMarkdown(_encounter->getText());
            else
                return;
        }
    }

    updateAnchors();
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
    QTextCharFormat format = ui->textBrowser->currentCharFormat();

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
            return;
        }
    }

    format.setAnchor(!newHRef.isEmpty());
    format.setAnchorHref(newHRef);
    format.setFontUnderline(!newHRef.isEmpty());
    ui->textBrowser->mergeCurrentCharFormat(format);
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

    if(_renderer)
    {
        _renderer->stop();
        _renderer->rewind();
    }
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

void EncounterTextEdit::playPause(bool play)
{
    if(_renderer)
        _renderer->playPause(play);
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

void EncounterTextEdit::setCodeView(bool active)
{
    if(active == _isCodeView)
        return;

    _isCodeView = active;
    setHtml();
    emit codeViewChanged(active);
}

void EncounterTextEdit::layerSelected(int selected)
{
    if(_encounter)
        _encounter->getLayerScene().setSelectedLayerIndex(selected);
}

void EncounterTextEdit::publishClicked(bool checked)
{
    if((!_encounter) || ((_isPublishing == checked) && (_renderer) && (_renderer->getObject() == _encounter)))
        return;

    _isPublishing = checked;

    if(_isPublishing)
    {
        if(!_renderer)
        {
            emit showPublishWindow();
            prepareImages();

            _renderer = new PublishGLTextRenderer(_encounter, _textImage);

            _renderer->setRotation(_rotation);
            connect(_renderer, &PublishGLTextRenderer::playPauseChanged, this, &EncounterTextEdit::playPauseChanged);
            connect(_renderer, &PublishGLTextRenderer::sceneSizeChanged, this, &EncounterTextEdit::sceneRectUpdated);
            emit registerRenderer(_renderer);
        }
    }
    else
    {
        _renderer = nullptr;
        disconnect(_renderer, &PublishGLTextRenderer::playPauseChanged, this, &EncounterTextEdit::playPauseChanged);
        disconnect(_renderer, &PublishGLTextRenderer::sceneSizeChanged, this, &EncounterTextEdit::sceneRectUpdated);
        emit registerRenderer(nullptr);
    }
}

void EncounterTextEdit::setRotation(int rotation)
{
    if(_rotation == rotation)
        return;

    _rotation = rotation;
    if(_renderer)
        _renderer->setRotation(_rotation);

    QTextDocument* doc = ui->textBrowser->document();
    if(doc)
        doc->setTextWidth(getRotatedTargetWidth());

    if((_isPublishing) && (_renderer))
    {
        prepareImages();
        _renderer->setTextImage(_textImage);
    }
}

void EncounterTextEdit::setBackgroundColor(const QColor& color)
{
    if(_encounter)
        _encounter->setBackgroundColor(color);
}

void EncounterTextEdit::editLayers()
{
    if(!_encounter)
        return;

    LayersEditDialog dlg(_encounter->getLayerScene());
    dlg.resize(width() * 9 / 10, height() * 9 / 10);
    dlg.exec();

    emit setLayers(_encounter->getLayerScene().getLayers(), _encounter->getLayerScene().getSelectedLayerIndex());
}

void EncounterTextEdit::updateAnchors()
{
    if (!_encounter)
        return;

    // Block signals to prevent unwanted textChanged() recursion
    ui->textBrowser->blockSignals(true);

    QTextCursor originalCursor = ui->textBrowser->textCursor();
    int originalScrollPos = ui->textBrowser->verticalScrollBar()->value();

    QTextCursor cursor = originalCursor;
    QRegularExpression regex("\\[\\[([^\\[\\]]+)\\]\\]");
    cursor.movePosition(QTextCursor::Start);  // Move only this temporary cursor
    ui->textBrowser->setTextCursor(cursor);

    while(ui->textBrowser->find(regex))
    {
        QTextCursor selectionCursor = ui->textBrowser->textCursor();
        int startPos = selectionCursor.selectionStart();
        int endPos = selectionCursor.selectionEnd();
        selectionCursor.setPosition(startPos + 2);
        selectionCursor.setPosition(endPos - 2, QTextCursor::KeepAnchor);

        if (!selectionCursor.selectedText().isEmpty())
        {
            Campaign* campaign = dynamic_cast<Campaign*>(_encounter->getParentByType(DMHelper::CampaignType_Campaign));
            if (campaign)
            {
                QTextCharFormat format = selectionCursor.charFormat();
                if (campaign->searchDirectChildrenByName(selectionCursor.selectedText()))
                {
                    format.setAnchor(true);
                    format.setAnchorHref(QString("DMHelper@") + selectionCursor.selectedText());
                    format.setFontItalic(true);
                    selectionCursor.mergeCharFormat(format);
                }
                else if (format.isAnchor())
                {
                    format.setAnchor(false);
                    format.setAnchorHref("");
                    format.setFontItalic(false);
                    selectionCursor.mergeCharFormat(format);
                }
            }
        }
    }

    // Restore cursor and scroll position
    ui->textBrowser->setTextCursor(originalCursor);
    ui->textBrowser->verticalScrollBar()->setValue(originalScrollPos);

    // Re-enable signals
    ui->textBrowser->blockSignals(false);
}

void EncounterTextEdit::storeEncounter()
{
    if(!_encounter)
        return;

    disconnect(_encounter, &EncounterText::textChanged, this, &EncounterTextEdit::readEncounter);

    if(_encounter->getObjectType() == DMHelper::CampaignType_Text)
    {
        if(_encounter->getTranslated())
            _encounter->setTranslatedText(toHtml());
        else
            _encounter->setText(toHtml());
    }
    else if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
    {
        _encounter->setText(toHtml());
    }

    connect(_encounter, &EncounterText::textChanged, this, &EncounterTextEdit::readEncounter);

}

void EncounterTextEdit::readEncounter()
{
    if(!_encounter)
        return;

    disconnect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(triggerEncounterChanged()));

    emit textWidthChanged(_encounter->getTextWidth());
    emit animatedChanged(_encounter->getAnimated());
    emit scrollSpeedChanged(_encounter->getScrollSpeed());
    emit translatedChanged(_encounter->getTranslated());
    emit backgroundColorChanged(_encounter->getBackgroundColor());

    bool showCodeView = false;
    _isCodeView = false;
    if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
    {
        EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
        if((linkedText->getFileType() == DMHelper::FileType_HTML) || (linkedText->getFileType() == DMHelper::FileType_Markdown))
        {
            showCodeView = true;
            emit codeViewChanged(_isCodeView);
        }
    }
    emit codeViewVisible(showCodeView);

    ui->textBrowser->setTextWidth(_encounter->getTextWidth());
    loadImage();

    setAnimated(_encounter->getAnimated());
    setTranslated(_encounter->getTranslated());
    setHtml();

    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(triggerEncounterChanged()));
}

void EncounterTextEdit::updateEncounter()
{
    if(!_encounter)
        return;

    disconnect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(triggerEncounterChanged()));
        bool showCodeView = false;
        if(_encounter->getObjectType() == DMHelper::CampaignType_LinkedText)
        {
            EncounterTextLinked* linkedText = dynamic_cast<EncounterTextLinked*>(_encounter);
            if((linkedText->getFileType() == DMHelper::FileType_HTML) || (linkedText->getFileType() == DMHelper::FileType_Markdown))
                showCodeView = true;
        }
        emit codeViewVisible(showCodeView);
        setHtml();
    connect(ui->textBrowser, SIGNAL(textChanged()), this, SLOT(triggerEncounterChanged()));
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

    _backgroundImage = _encounter->getLayerScene().mergedImage();

    scaleBackgroundImage();
    setPublishCheckable();
}

void EncounterTextEdit::handleLayersChanged()
{
    if(!_encounter)
        return;

    emit setLayers(_encounter->getLayerScene().getLayers(), _encounter->getLayerScene().getSelectedLayerIndex());
}

void EncounterTextEdit::refreshImage()
{
    loadImage();
    update();
}

void EncounterTextEdit::triggerEncounterChanged()
{
    if(_encounterChangedTimer)
        killTimer(_encounterChangedTimer);

    _encounterChangedTimer = startTimer(ENCOUNTERTEXTEDIT_STORE_INTERVAL);
}

void EncounterTextEdit::triggerUpdateAnchor()
{
    if(_updateAnchorTimer)
        killTimer(_updateAnchorTimer);

    _updateAnchorTimer = startTimer(ENCOUNTERTEXTEDIT_ANCHOR_UPDATE_INTERVAL);
}

void EncounterTextEdit::sceneRectUpdated(const QSize& size)
{
    if(size == _targetSize)
        return;

    _targetSize = size;
    QTextDocument* doc = ui->textBrowser->document();
    if(doc)
        doc->setTextWidth(getRotatedTargetWidth());

    if((_isPublishing) && (_renderer))
    {
        prepareImages();
        _renderer->setTextImage(_textImage);
    }
}

void EncounterTextEdit::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    scaleBackgroundImage();
}

void EncounterTextEdit::timerEvent(QTimerEvent *event)
{
    if((!event) || (event->timerId() == 0))
        return;

    if(event->timerId() == _encounterChangedTimer)
    {
        killTimer(_encounterChangedTimer);
        _encounterChangedTimer = 0;
        storeEncounter();
    }

    if(event->timerId() == _updateAnchorTimer)
    {
        killTimer(_updateAnchorTimer);
        _updateAnchorTimer = 0;
        updateAnchors();
    }
}

void EncounterTextEdit::scaleBackgroundImage()
{
    if(!_backgroundImage.isNull())
        _backgroundImageScaled = _backgroundImage.scaledToWidth(ui->textBrowser->width(), Qt::FastTransformation);
}

void EncounterTextEdit::prepareImages()
{
    if((!_encounter) || (_targetSize.isEmpty()))
        return;

    if(_backgroundImage.isNull())
    {
        _prescaledImage = QImage(getRotatedTargetSize(), QImage::Format_ARGB32_Premultiplied);
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

    _textImage = getDocumentTextImage(_prescaledImage.width());
}

QImage EncounterTextEdit::getDocumentTextImage(int renderWidth)
{
    QImage result;

    if(renderWidth <= 0)
        return result;

    QTextDocument* doc = ui->textBrowser->document();
    if(doc)
    {
        int oldTextWidth = doc->textWidth();
        int textPercentage = _encounter ? _encounter->getTextWidth() : 100;
        int absoluteWidth = renderWidth * textPercentage / 100;

        doc->setTextWidth(absoluteWidth);

        result = QImage(absoluteWidth, doc->size().height(), QImage::Format_ARGB32_Premultiplied);
        result.fill(Qt::transparent);
        QPainter painter;
        painter.begin(&result);
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

void EncounterTextEdit::setPublishCheckable()
{
    emit checkableChanged(_encounter ? _encounter->getAnimated() : false);
}

QSize EncounterTextEdit::getRotatedTargetSize()
{
    return (_rotation % 180 == 0) ? _targetSize : _targetSize.transposed();
}

int EncounterTextEdit::getRotatedTargetWidth()
{
    return (_rotation % 180 == 0) ? _targetSize.width() : _targetSize.height();
}

void EncounterTextEdit::cancelTimers()
{
    if(_encounterChangedTimer)
    {
        killTimer(_encounterChangedTimer);
        _encounterChangedTimer = 0;
    }

    if(_updateAnchorTimer)
    {
        killTimer(_updateAnchorTimer);
        _updateAnchorTimer = 0;
    }
}
