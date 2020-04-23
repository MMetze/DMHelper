#include "encountertextedit.h"
#include "encountertext.h"
#include "ui_encountertextedit.h"
#include <QKeyEvent>
#include <QTextCharFormat>
#include <QDebug>

EncounterTextEdit::EncounterTextEdit(QWidget *parent) :
    CampaignObjectFrame(parent),
    ui(new Ui::EncounterTextEdit),
    _keys(),
    _encounter(nullptr),
    _formatter(new TextEditFormatterFrame(this))
{
    ui->setupUi(this);

    ui->textBrowser->viewport()->setCursor(Qt::IBeamCursor);

    //    connect(ui->textBrowser, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
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
    //ui->textFormatter->setTextEdit(ui->textBrowser);
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
    emit setPublishEnabled(false);
}

void EncounterTextEdit::deactivateObject()
{
    if(!_encounter)
    {
        qDebug() << "[EncounterTextEdit] WARNING: Invalid (nullptr) text encounter object deactivated!";
        return;
    }

    storeEncounter();
    setEncounter(nullptr);
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
    if(_encounter != encounter)
    {
        storeEncounter();
        _encounter = encounter;
        readEncounter();
    }
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
    if((watched != ui->textBrowser) || (event->type() != QEvent::KeyPress))
        return false;

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
