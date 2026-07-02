#include "templatefieldformat.h"
#include "templateobject.h"
#include "templatefactory.h"
#include "combatant.h"
#include <QIntValidator>
#include <QStringList>
#include <QChar>
#include <QDebug>

namespace
{
    // ---- Format spec internals ----------------------------------------------

    QString unquote(QString text)
    {
        if((text.size() >= 2) && (text.startsWith(QLatin1Char('"'))) && (text.endsWith(QLatin1Char('"'))))
            return text.mid(1, text.size() - 2);
        return text;
    }

    bool parseRange(const QString& body, int& outMin, int& outMax)
    {
        const int dotDot = body.indexOf(QStringLiteral(".."));
        if(dotDot < 0)
            return false;

        bool okMin = false;
        bool okMax = false;
        const int parsedMin = body.left(dotDot).trimmed().toInt(&okMin);
        const int parsedMax = body.mid(dotDot + 2).trimmed().toInt(&okMax);
        if((!okMin) || (!okMax))
            return false;

        outMin = parsedMin;
        outMax = parsedMax;
        return true;
    }

    QString formatPaddedInt(int value, int padDigits, bool signedDisplay)
    {
        const int absValue = qAbs(value);
        QString digits = QString::number(absValue);
        if(padDigits > digits.size())
            digits = QString(padDigits - digits.size(), QLatin1Char('0')) + digits;

        if(signedDisplay)
            return (value < 0 ? QStringLiteral("-") : QStringLiteral("+")) + digits;

        if(value < 0)
            return QStringLiteral("-") + digits;
        return digits;
    }
}

namespace TemplateFieldFormat
{

FormatSpec parseFormat(const QString& spec)
{
    FormatSpec result;
    if(spec.isEmpty())
        return result;

    const QStringList parts = spec.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for(const QString& rawPart : parts)
    {
        const QString part = rawPart.trimmed();
        if(part.isEmpty())
            continue;

        const int colon = part.indexOf(QLatin1Char(':'));
        const QString keyword = (colon < 0 ? part : part.left(colon)).trimmed().toLower();
        const QString body = (colon < 0 ? QString() : part.mid(colon + 1).trimmed());

        if(keyword == QStringLiteral("int"))
        {
            result.isInt = true;
            if(!body.isEmpty())
            {
                int lo = 0;
                int hi = 0;
                if(parseRange(body, lo, hi))
                {
                    result.hasRange = true;
                    result.rangeMin = lo;
                    result.rangeMax = hi;
                }
            }
        }
        else if(keyword == QStringLiteral("signed"))
        {
            result.isInt = true;
            result.isSigned = true;
            if(!body.isEmpty())
            {
                int lo = 0;
                int hi = 0;
                if(parseRange(body, lo, hi))
                {
                    result.hasRange = true;
                    result.rangeMin = lo;
                    result.rangeMax = hi;
                }
            }
        }
        else if(keyword == QStringLiteral("prefix"))
        {
            result.prefix = unquote(body);
        }
        else if(keyword == QStringLiteral("suffix"))
        {
            result.suffix = unquote(body);
        }
        else if(keyword == QStringLiteral("pad"))
        {
            bool ok = false;
            const int n = body.toInt(&ok);
            if((ok) && (n > 0))
                result.padDigits = n;
        }
        else if(keyword == QStringLiteral("resource"))
        {
            if(body == QStringLiteral("current"))
                result.resourceMode = FormatSpec::ResourceMode_Current;
            else if(body == QStringLiteral("max"))
                result.resourceMode = FormatSpec::ResourceMode_Max;
            else
                result.resourceMode = FormatSpec::ResourceMode_Both;
        }
        else if(keyword == QStringLiteral("resourcecurrent"))
        {
            result.resourceMode = FormatSpec::ResourceMode_Current;
        }
        else if(keyword == QStringLiteral("resourcemax"))
        {
            result.resourceMode = FormatSpec::ResourceMode_Max;
        }
        else
        {
            qDebug() << "[TemplateFieldFormat] WARNING: Unknown format keyword:" << keyword << "in spec:" << spec;
        }
    }

    return result;
}

QString applyFormatInt(int rawValue, const FormatSpec& spec)
{
    QString core;
    if((spec.padDigits > 0) || (spec.isSigned))
        core = formatPaddedInt(rawValue, spec.padDigits, spec.isSigned);
    else
        core = QString::number(rawValue);

    return spec.prefix + core + spec.suffix;
}

QString applyFormat(const QString& rawValue, const FormatSpec& spec)
{
    if(spec.isEmpty())
        return rawValue;

    if(spec.isInt)
    {
        bool ok = false;
        const int n = rawValue.toInt(&ok);
        if(!ok)
            return spec.prefix + rawValue + spec.suffix;
        return applyFormatInt(n, spec);
    }

    return spec.prefix + rawValue + spec.suffix;
}

QString stripFormat(const QString& displayValue, const FormatSpec& spec)
{
    QString result = displayValue;

    if((!spec.prefix.isEmpty()) && (result.startsWith(spec.prefix)))
        result = result.mid(spec.prefix.size());
    if((!spec.suffix.isEmpty()) && (result.endsWith(spec.suffix)))
        result.chop(spec.suffix.size());

    result = result.trimmed();

    // Strip a redundant leading '+' so the model stores a plain integer.
    if((spec.isSigned) && (result.startsWith(QLatin1Char('+'))))
        result = result.mid(1);

    return result;
}

QValidator* makeValidator(const FormatSpec& spec, QObject* parent)
{
    if(!spec.isInt)
        return nullptr;

    const int lo = spec.hasRange ? spec.rangeMin : INT_MIN;
    const int hi = spec.hasRange ? spec.rangeMax : INT_MAX;
    return new QIntValidator(lo, hi, parent);
}

}  // namespace TemplateFieldFormat


// ===========================================================================
// ComputeExpr
// ===========================================================================

ComputeExpr::ComputeExpr() = default;

QStringList ComputeExpr::tokenize(const QString& expression)
{
    QStringList tokens;
    int i = 0;
    const int n = expression.size();
    while(i < n)
    {
        const QChar ch = expression.at(i);
        if(ch.isSpace())
        {
            ++i;
            continue;
        }

        if((ch == QLatin1Char('+')) || (ch == QLatin1Char('-')) ||
           (ch == QLatin1Char('*')) || (ch == QLatin1Char('/')) ||
           (ch == QLatin1Char('(')) || (ch == QLatin1Char(')')))
        {
            tokens.append(QString(ch));
            ++i;
            continue;
        }

        if(ch.isDigit())
        {
            int j = i;
            while((j < n) && (expression.at(j).isDigit()))
                ++j;
            tokens.append(expression.mid(i, j - i));
            i = j;
            continue;
        }

        if((ch.isLetter()) || (ch == QLatin1Char('_')))
        {
            int j = i;
            while((j < n) && ((expression.at(j).isLetterOrNumber()) || (expression.at(j) == QLatin1Char('_'))))
                ++j;
            tokens.append(expression.mid(i, j - i));
            i = j;
            continue;
        }

        // Unrecognised character — bail out, parse will fail.
        return QStringList();
    }
    return tokens;
}

int ComputeExpr::addNode(const Node& node)
{
    _nodes.append(node);
    return _nodes.size() - 1;
}

ComputeExpr ComputeExpr::parse(const QString& expression)
{
    ComputeExpr result;
    if(expression.trimmed().isEmpty())
        return result;

    const QStringList tokens = tokenize(expression);
    if(tokens.isEmpty())
        return result;

    int pos = 0;
    const int rootIndex = result.parseExpr(tokens, pos);
    if((rootIndex < 0) || (pos != tokens.size()))
        return result;

    result._root = rootIndex;
    result._valid = true;
    return result;
}

int ComputeExpr::parseExpr(const QStringList& tokens, int& pos)
{
    int left = parseTerm(tokens, pos);
    if(left < 0)
        return -1;

    while(pos < tokens.size())
    {
        const QString& tok = tokens.at(pos);
        if((tok != QStringLiteral("+")) && (tok != QStringLiteral("-")))
            break;
        ++pos;
        const int right = parseTerm(tokens, pos);
        if(right < 0)
            return -1;
        Node n;
        n.kind = (tok == QStringLiteral("+")) ? Node::Add : Node::Sub;
        n.left = left;
        n.right = right;
        left = addNode(n);
    }
    return left;
}

int ComputeExpr::parseTerm(const QStringList& tokens, int& pos)
{
    int left = parseFactor(tokens, pos);
    if(left < 0)
        return -1;

    while(pos < tokens.size())
    {
        const QString& tok = tokens.at(pos);
        if((tok != QStringLiteral("*")) && (tok != QStringLiteral("/")))
            break;
        ++pos;
        const int right = parseFactor(tokens, pos);
        if(right < 0)
            return -1;
        Node n;
        n.kind = (tok == QStringLiteral("*")) ? Node::Mul : Node::Div;
        n.left = left;
        n.right = right;
        left = addNode(n);
    }
    return left;
}

int ComputeExpr::parseFactor(const QStringList& tokens, int& pos)
{
    if(pos >= tokens.size())
        return -1;

    const QString& tok = tokens.at(pos);
    if((tok == QStringLiteral("+")) || (tok == QStringLiteral("-")))
    {
        ++pos;
        const int operand = parseFactor(tokens, pos);
        if(operand < 0)
            return -1;
        if(tok == QStringLiteral("+"))
            return operand;  // unary plus is a no-op
        Node n;
        n.kind = Node::Neg;
        n.left = operand;
        return addNode(n);
    }
    return parsePrimary(tokens, pos);
}

int ComputeExpr::parsePrimary(const QStringList& tokens, int& pos)
{
    if(pos >= tokens.size())
        return -1;

    const QString tok = tokens.at(pos);

    if(tok == QStringLiteral("("))
    {
        ++pos;
        const int inner = parseExpr(tokens, pos);
        if(inner < 0)
            return -1;
        if((pos >= tokens.size()) || (tokens.at(pos) != QStringLiteral(")")))
            return -1;
        ++pos;
        return inner;
    }

    bool ok = false;
    const int literalValue = tok.toInt(&ok);
    if(ok)
    {
        ++pos;
        Node n;
        n.kind = Node::Literal;
        n.literal = literalValue;
        return addNode(n);
    }

    // Identifier
    if((!tok.isEmpty()) && ((tok.at(0).isLetter()) || (tok.at(0) == QLatin1Char('_'))))
    {
        ++pos;
        Node n;
        n.kind = Node::Reference;
        n.name = tok;
        _references.insert(tok);
        return addNode(n);
    }

    return -1;
}

int ComputeExpr::evaluate(const TemplateObject& source) const
{
    if((!_valid) || (_root < 0))
        return 0;
    return evalNode(_root, source);
}

int ComputeExpr::evalNode(int index, const TemplateObject& source) const
{
    if((index < 0) || (index >= _nodes.size()))
        return 0;

    const Node& n = _nodes.at(index);
    switch(n.kind)
    {
    case Node::Literal:
        return n.literal;
    case Node::Reference:
    {
        TemplateFactory* factory = source.getFactory();
        if((factory) && (factory->hasAttribute(n.name)))
        {
            const DMHAttribute attr = factory->getAttribute(n.name);
            if(attr._type == TemplateFactory::TemplateType_resource)
                return source.getResourceValue(n.name).first;
            if(attr._type == TemplateFactory::TemplateType_boolean)
                return source.getBoolValue(n.name) ? 1 : 0;
        }
        // Fall-through: integer or unknown — getIntValue handles either.
        return source.getIntValue(n.name);
    }
    case Node::Neg:
        return -evalNode(n.left, source);
    case Node::Pos:
        return evalNode(n.left, source);
    case Node::Add:
        return evalNode(n.left, source) + evalNode(n.right, source);
    case Node::Sub:
        return evalNode(n.left, source) - evalNode(n.right, source);
    case Node::Mul:
        return evalNode(n.left, source) * evalNode(n.right, source);
    case Node::Div:
    {
        const int divisor = evalNode(n.right, source);
        if(divisor == 0)
            return 0;
        return evalNode(n.left, source) / divisor;
    }
    }
    return 0;
}
