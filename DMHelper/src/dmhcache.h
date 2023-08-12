#ifndef DMHCACHE_H
#define DMHCACHE_H

#include <QString>

class DMHCache
{
public:
    explicit DMHCache();

    QString getCacheFilePath(const QString& filename, const QString& fileExtension = QString());
    void ensureCacheExists();
    QString getCachePath();

protected:
    QString getCacheFileName(const QString& filename);

};

#endif // DMHCACHE_H
