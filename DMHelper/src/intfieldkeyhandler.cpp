#include "intfieldkeyhandler.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QMetaObject>

IntFieldKeyHandler::IntFieldKeyHandler(QLineEdit* lineEdit, const FormatSpec& spec) :
    QObject(lineEdit),
    _lineEdit(lineEdit),
    _spec(spec)
{
    if(_lineEdit)
        _lineEdit->installEventFilter(this);
}

bool IntFieldKeyHandler::eventFilter(QObject* watched, QEvent* event)
{
    if((watched != _lineEdit) || (!event) || (event->type() != QEvent::KeyPress))
        return QObject::eventFilter(watched, event);

    // Read-only line edits never accept ad-hoc adjustments \u2014 the value is
    // entirely driven by an upstream computation or by another widget.
    if((!_lineEdit) || (_lineEdit->isReadOnly()))
        return QObject::eventFilter(watched, event);

    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    int delta = 0;
    switch(keyEvent->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Plus:
            delta = +1;
            break;
        case Qt::Key_Down:
        case Qt::Key_Minus:
            delta = -1;
            break;
        default:
            return QObject::eventFilter(watched, event);
    }

    // For "+" / "-" we only intercept when the field is empty or already a
    // pure integer; otherwise the keys are part of normal signed-number entry
    // and must reach the line edit. Up / Down arrows always intercept since
    // they have no other meaning inside a single-line edit.
    if((keyEvent->key() == Qt::Key_Plus) || (keyEvent->key() == Qt::Key_Minus))
    {
        const QString stripped = TemplateFieldFormat::stripFormat(_lineEdit->text(), _spec);
        bool ok = false;
        stripped.trimmed().toInt(&ok);
        if((!stripped.trimmed().isEmpty()) && (!ok))
            return QObject::eventFilter(watched, event);
    }

    adjustBy(delta);
    return true;
}

void IntFieldKeyHandler::adjustBy(int delta)
{
    if(!_lineEdit)
        return;

    const QString stripped = TemplateFieldFormat::stripFormat(_lineEdit->text(), _spec);
    bool ok = false;
    int value = stripped.trimmed().toInt(&ok);
    if(!ok)
        value = 0;

    value += delta;

    if(_spec.hasRange)
    {
        if(value < _spec.rangeMin)
            value = _spec.rangeMin;
        else if(value > _spec.rangeMax)
            value = _spec.rangeMax;
    }

    _lineEdit->setText(TemplateFieldFormat::applyFormatInt(value, _spec));
    _lineEdit->setCursorPosition(0);

    // Trigger the existing editingFinished wiring (set up in TemplateFactory)
    // so the new value is written back to the model immediately rather than
    // waiting for the user to tab out of the field. editingFinished has no
    // public emitter; the meta-object invocation is the standard idiom.
    QMetaObject::invokeMethod(_lineEdit, "editingFinished", Qt::DirectConnection);
}
