#include "encountertextedit.h"
#include "ui_encountertextedit.h"
#include <QKeyEvent>
#include <QTextCharFormat>

EncounterTextEdit::EncounterTextEdit(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::EncounterTextEdit),
    _keys(),
    _encounter(nullptr)
{
    ui->setupUi(this);

    ui->textBrowser->viewport()->setCursor(Qt::IBeamCursor);

    connect(ui->textBrowser, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(ui->textBrowser, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));

    ui->textBrowser->installEventFilter(this);
    ui->textFormatter->setTextEdit(ui->textBrowser);
}

EncounterTextEdit::~EncounterTextEdit()
{
    delete ui;
}

void EncounterTextEdit::setKeys(QList<QString> keys)
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
    _encounter = encounter;
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
