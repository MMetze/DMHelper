#ifndef TEMPLATEFIELDFORMAT_H
#define TEMPLATEFIELDFORMAT_H

#include <QString>
#include <QStringList>
#include <QSet>

class QValidator;
class QObject;
class TemplateObject;

// ---------------------------------------------------------------------------
// dmhFormat spec parser
//
// Spec syntax: zero or more semicolon-separated tokens.
//  - "int"                   : require integer input
//  - "int:<min>..<max>"      : require integer in [min,max]
//  - "signed"                : prepend "+" for non-negative on display, parse signed input
//  - "signed:<min>..<max>"   : signed display + integer-range validation
//  - "prefix:<text>"         : decorative prefix wrapped around displayed value
//  - "suffix:<text>"         : decorative suffix wrapped around displayed value
//  - "pad:<n>"               : zero-pad numeric values to <n> digits (after sign)
//
// Tokens are case-insensitive on the keyword. The display helpers strip
// any prefix/suffix/sign before write-back so the raw value reaches the model.
// ---------------------------------------------------------------------------
struct FormatSpec
{
    bool isInt = false;
    bool isSigned = false;
    bool hasRange = false;
    int rangeMin = 0;
    int rangeMax = 0;
    int padDigits = 0;
    QString prefix;
    QString suffix;

    bool isEmpty() const { return !isInt && !isSigned && padDigits == 0 && prefix.isEmpty() && suffix.isEmpty(); }
};

namespace TemplateFieldFormat
{
    FormatSpec parseFormat(const QString& spec);

    // Format a raw model value for display. Numeric specs go through the
    // integer formatter; pure-text specs simply wrap with prefix/suffix.
    QString applyFormat(const QString& rawValue, const FormatSpec& spec);
    QString applyFormatInt(int rawValue, const FormatSpec& spec);

    // Inverse of applyFormat: given the user-entered text, strip cosmetic
    // affixes so the underlying model receives a clean value.
    QString stripFormat(const QString& displayValue, const FormatSpec& spec);

    // Returns a freshly allocated validator suitable for QLineEdit, or nullptr
    // if the spec does not require validation. Caller (or QLineEdit::setValidator)
    // takes ownership; pass parent to tie the lifetime to a widget.
    QValidator* makeValidator(const FormatSpec& spec, QObject* parent);
}

// ---------------------------------------------------------------------------
// dmhCompute expression parser/evaluator
//
// Grammar:
//   expr       := term (("+"|"-") term)*
//   term       := factor (("*"|"/") factor)*
//   factor     := ["+"|"-"] primary
//   primary    := INTEGER | IDENT | "(" expr ")"
//
// IDENT references a named field on the active TemplateObject. Lookups go
// through TemplateObject::getIntValue (boolean attributes return 0/1; resource
// attributes return the current count via getResourceValue().first).
//
// Failure modes (parse error, missing reference, divide-by-zero) all evaluate
// to 0 silently. No exceptions, no crashes.
// ---------------------------------------------------------------------------
class ComputeExpr
{
public:
    ComputeExpr();

    static ComputeExpr parse(const QString& expression);

    bool isValid() const { return _valid; }
    int evaluate(const TemplateObject& source) const;

    // Set of field names referenced by this expression. Used so computed
    // widgets can subscribe only to the relevant TemplateObject change events.
    const QSet<QString>& references() const { return _references; }

private:
    struct Node
    {
        enum Kind { Literal, Reference, Add, Sub, Mul, Div, Neg, Pos };
        Kind kind = Literal;
        int literal = 0;
        QString name;
        int left = -1;   // index into _nodes
        int right = -1;
    };

    int parseExpr(const QStringList& tokens, int& pos);
    int parseTerm(const QStringList& tokens, int& pos);
    int parseFactor(const QStringList& tokens, int& pos);
    int parsePrimary(const QStringList& tokens, int& pos);

    int evalNode(int index, const TemplateObject& source) const;
    int addNode(const Node& node);

    static QStringList tokenize(const QString& expression);

    QList<Node> _nodes;
    int _root = -1;
    bool _valid = false;
    QSet<QString> _references;
};

#endif // TEMPLATEFIELDFORMAT_H
