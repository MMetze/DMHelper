#ifndef DMHCACHE_H
#define DMHCACHE_H

#include <QString>

class DMHCache
{
public:
    explicit DMHCache();

    QString getCacheFilePath(const QString& filename, const QString& fileExtension = QString(), const QString& subdirectory = QString());
    void ensureCacheExists(const QString& subdirectory = QString());
    QString getCachePath(const QString& subdirectory = QString());

protected:
    QString getCacheFileName(const QString& filename);

};

#endif // DMHCACHE_H
