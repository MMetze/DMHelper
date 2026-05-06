#ifndef INTFIELDKEYHANDLER_H
#define INTFIELDKEYHANDLER_H

#include "templatefieldformat.h"
#include <QObject>

class QLineEdit;

// ---------------------------------------------------------------------------
// IntFieldKeyHandler
//
// Event filter for QLineEdit fields whose dmhFormat resolves to an integer
// spec. Translates Up / Down / "+" / "-" key presses into ±1 adjustments on
// the underlying value, respecting any range clamp declared by the spec.
//
// The handler does its own format strip / re-apply round trip so the visible
// text stays consistent with the format (signed prefix, padding, prefix /
// suffix decoration). After updating the displayed text it triggers the host
// QLineEdit's editingFinished signal so the existing write-back wiring
// installed by TemplateFactory propagates the new value to the model in a
// single atomic step.
//
// One handler instance is created per line edit and parented to it so its
// lifetime matches the widget. The handler is not used for compute (read-
// only) fields \u2014 those are filtered out at the install site.
// ---------------------------------------------------------------------------
class IntFieldKeyHandler : public QObject
{
    Q_OBJECT
public:
    IntFieldKeyHandler(QLineEdit* lineEdit, const FormatSpec& spec);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void adjustBy(int delta);

    QLineEdit* _lineEdit;
    FormatSpec _spec;
};

#endif // INTFIELDKEYHANDLER_H
