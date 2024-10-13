#include "globalsearch.h"

const int GLOBALSEARCH_EXTRA_TEXT = 20;

bool GlobalSearch_Interface::compareStringValue(const QString& sourceString, const QString& searchString, QString& result)
{
    qsizetype pos = sourceString.indexOf(searchString, 0, Qt::CaseInsensitive);
    if(pos == -1)
        return false;

    if(pos > GLOBALSEARCH_EXTRA_TEXT)
        result = QString("...");

    result += sourceString.mid(pos - GLOBALSEARCH_EXTRA_TEXT, searchString.length() + (2 * GLOBALSEARCH_EXTRA_TEXT));

    if(sourceString.length() > pos + searchString.length() + GLOBALSEARCH_EXTRA_TEXT)
        result += QString("...");

    result.remove(QChar::LineFeed);

    return true;
}
