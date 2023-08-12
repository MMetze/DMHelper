#include "dmhcache.h"
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

DMHCache::DMHCache()
{
}

QString DMHCache::getCacheFilePath(const QString& filename, const QString& fileExtension)
{
    QString cacheFile = getCacheFileName(filename);
    if(cacheFile.isEmpty())
        return QString();

    return getCachePath() + cacheFile + (fileExtension.isEmpty() ? QString() : (QString(".") + fileExtension));
}

void DMHCache::ensureCacheExists()
{
    QString standardPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if((standardPath.isEmpty()) || (!QFile::exists(standardPath)))
    {
        qDebug() << "[DMHCache] ERROR: Unable to find standard writable location for the DMH Application: " << standardPath;
        return;
    }

    QDir cacheDir(getCachePath());
    if(!cacheDir.exists())
    {
        if(QDir().mkpath(cacheDir.absolutePath()))
            qDebug() << "[DMHCache] Created cache directory: " << cacheDir.absolutePath();
        else
            qDebug() << "[DMHCache] ERROR: Could not create cache directory: " << cacheDir.absolutePath();
    }
}

QString DMHCache::getCachePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QString("/cache/");
}

QString DMHCache::getCacheFileName(const QString& filename)
{
    if(filename.isEmpty())
        return QString();

    return QCryptographicHash::hash(filename.toUtf8(), QCryptographicHash::Md5).toHex(0);
}

