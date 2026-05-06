#include "battledialogeventresetresources.h"
#include "dmconstants.h"
#include <QDomElement>

static const char* XMLATTR_SCOPE = "scope";

BattleDialogEventResetResources::BattleDialogEventResetResources(const QString& scope) :
    BattleDialogEvent(),
    _scope(scope)
{
}

BattleDialogEventResetResources::BattleDialogEventResetResources(const QDomElement& element) :
    BattleDialogEvent(element),
    _scope(element.attribute(QString::fromLatin1(XMLATTR_SCOPE)))
{
}

BattleDialogEventResetResources::BattleDialogEventResetResources(const BattleDialogEventResetResources& other) :
    BattleDialogEvent(other),
    _scope(other._scope)
{
}

BattleDialogEventResetResources::~BattleDialogEventResetResources()
{
}

int BattleDialogEventResetResources::getType() const
{
    return DMHelper::BattleEvent_ResetResources;
}

BattleDialogEvent* BattleDialogEventResetResources::clone()
{
    return new BattleDialogEventResetResources(*this);
}

QDomElement BattleDialogEventResetResources::outputXML(QDomElement &element, bool isExport)
{
    Q_UNUSED(isExport);
    if(!_scope.isEmpty())
        element.setAttribute(QString::fromLatin1(XMLATTR_SCOPE), _scope);
    return element;
}

QString BattleDialogEventResetResources::getScope() const
{
    return _scope;
}

void BattleDialogEventResetResources::setScope(const QString& scope)
{
    _scope = scope;
}
