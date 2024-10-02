#include "globalsearch.h"

bool GlobalSearch_Interface::compareStringValue(const QString& sourceString, const QString& searchString, QString& result)
{
    qsizetype pos = sourceString.indexOf(searchString, 0, Qt::CaseInsensitive);
    if(pos == -1)
        return false;

    if(pos > 10)
        result = QString("...");

    result += sourceString.mid(pos - 10, searchString.length() + 20);

    if(sourceString.length() > pos + searchString.length() + 10)
        result += QString("...");

    return true;
}
