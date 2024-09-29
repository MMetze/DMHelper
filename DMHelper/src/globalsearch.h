#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include <QStringList>

class GlobalSearch_Interface
{
public:
    virtual QStringList search(const QString& searchString) = 0;
    // TODO: add a "getDetails" interface
    // TODO: add a bool search call
};


#endif // GLOBALSEARCH_H
