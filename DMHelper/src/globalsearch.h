#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include <QStringList>

class GlobalSearch_Interface
{
public:
    virtual QStringList search(const QString& searchString) = 0;
    // TODO: add a "getDetails" interface
    // TODO: add a bool search call

    static bool compareStringValue(const QString& sourceString, const QString& searchString, QString& result);
};


#endif // GLOBALSEARCH_H
