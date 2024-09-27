#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include <QStringList>

class GlobalSearch_Interface
{
public:
    virtual QStringList search(const QString& searchString) = 0;
};


#endif // GLOBALSEARCH_H
