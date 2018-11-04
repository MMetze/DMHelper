#include "encountertextedit.h"
#include <QKeyEvent>
#include <QTextCharFormat>

EncounterTextEdit::EncounterTextEdit(QWidget *parent) :
    QTextBrowser(parent),
    _keys(),
    _encounter(0)
{
    setReadOnly(false);
    setOpenExternalLinks(false);
    setOpenLinks(false);
    setTextInteractionFlags(textInteractionFlags() | Qt::LinksAccessibleByMouse);
    viewport()->setCursor(Qt::IBeamCursor);
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

void EncounterTextEdit::clear()
{
    _keys.clear();
    setPlainText(QString(""));
}

void EncounterTextEdit::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
        case Qt::Key_B:
            if( (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
            {
                QTextCharFormat format = currentCharFormat();
                format.setFontWeight(format.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                setCurrentCharFormat(format);
                e->accept();
                return;
            }
            break;

        case Qt::Key_I:
            if( (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
            {
                QTextCharFormat format = currentCharFormat();
                format.setFontItalic(!format.fontItalic());
                setCurrentCharFormat(format);
                e->accept();
                return;
            }
            break;

        case Qt::Key_U:
            if( (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
            {
                QTextCharFormat format = currentCharFormat();
                format.setFontUnderline(!format.fontUnderline());
                setCurrentCharFormat(format);
                e->accept();
                return;
            }
            break;

        default:
            break;
    }

    QTextEdit::keyPressEvent(e);
}
