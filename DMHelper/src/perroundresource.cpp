#include "perroundresource.h"
#include <QDomDocument>
#include <QDomElement>

const char* PerRoundResource::RECHARGE_TURN       = "turn";
const char* PerRoundResource::RECHARGE_ROUND      = "round";
const char* PerRoundResource::RECHARGE_SHORT_REST = "shortRest";
const char* PerRoundResource::RECHARGE_LONG_REST  = "longRest";

const char* PerRoundResource::KEY_NAME     = "name";
const char* PerRoundResource::KEY_MAX      = "max";
const char* PerRoundResource::KEY_RECHARGE = "recharge";

static const char* XMLATTR_NAME     = "name";
static const char* XMLATTR_MAX      = "max";
static const char* XMLATTR_CURRENT  = "current";
static const char* XMLATTR_RECHARGE = "recharge";

PerRoundResource::PerRoundResource() :
    name(),
    max(0),
    recharge(QString::fromLatin1(RECHARGE_ROUND))
{
}

PerRoundResource::PerRoundResource(const QString& name, int max, const QString& recharge) :
    name(name),
    max(max),
    recharge(recharge)
{
}

QHash<QString, QVariant> PerRoundResource::toHash() const
{
    QHash<QString, QVariant> result;
    result.insert(QString::fromLatin1(KEY_NAME), name);
    result.insert(QString::fromLatin1(KEY_MAX), max);
    result.insert(QString::fromLatin1(KEY_RECHARGE), recharge);
    return result;
}

PerRoundResource PerRoundResource::fromHash(const QHash<QString, QVariant>& hash)
{
    PerRoundResource result;
    result.name = hash.value(QString::fromLatin1(KEY_NAME)).toString();
    result.max = hash.value(QString::fromLatin1(KEY_MAX)).toInt();
    result.recharge = hash.value(QString::fromLatin1(KEY_RECHARGE), QString::fromLatin1(RECHARGE_ROUND)).toString();
    return result;
}

QDomElement PerRoundResource::writeXML(QDomDocument& doc, const QString& elementName, int currentValue) const
{
    QDomElement element = doc.createElement(elementName);
    element.setAttribute(QString::fromLatin1(XMLATTR_NAME), name);
    element.setAttribute(QString::fromLatin1(XMLATTR_MAX), max);
    element.setAttribute(QString::fromLatin1(XMLATTR_CURRENT), currentValue);
    element.setAttribute(QString::fromLatin1(XMLATTR_RECHARGE), recharge);
    return element;
}

PerRoundResource PerRoundResource::readXML(const QDomElement& element, int* outCurrentValue)
{
    PerRoundResource result;
    result.name = element.attribute(QString::fromLatin1(XMLATTR_NAME));
    result.max = element.attribute(QString::fromLatin1(XMLATTR_MAX), QStringLiteral("0")).toInt();
    result.recharge = element.attribute(QString::fromLatin1(XMLATTR_RECHARGE), QString::fromLatin1(RECHARGE_ROUND));

    if(outCurrentValue)
        *outCurrentValue = element.attribute(QString::fromLatin1(XMLATTR_CURRENT), QStringLiteral("0")).toInt();

    return result;
}
