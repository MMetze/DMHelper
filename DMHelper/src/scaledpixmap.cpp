#include "scaledpixmap.h"
#include "dmconstants.h"

#include <QFile>
#include <QDebug>

ScaledPixmap* ScaledPixmap::_defaultPixmap = nullptr;

ScaledPixmap::ScaledPixmap() :
    _pixmaps(DMHelper::PixmapSize_Count),
    _basePixmap()
{
}

ScaledPixmap::ScaledPixmap(const ScaledPixmap &obj) :
    _pixmaps(obj._pixmaps),
    _basePixmap(obj._basePixmap)
{
}

ScaledPixmap::~ScaledPixmap()
{
}

ScaledPixmap* ScaledPixmap::defaultPixmap()
{
    if(!_defaultPixmap)
    {
        _defaultPixmap = new ScaledPixmap();
        _defaultPixmap->setBasePixmap(QString(":/img/data/portrait.png"));
    }

    return _defaultPixmap;
}

void ScaledPixmap::cleanupDefaultPixmap()
{
    delete _defaultPixmap;
    _defaultPixmap = nullptr;
}

bool ScaledPixmap::setBasePixmap(const QString& basePixmap)
{
    if(_basePixmap == basePixmap)
        return false;

    if(!QFile::exists(basePixmap))
    {
        qDebug() << "[ScaledPixmap] ERROR: Base Pixmap file not found: " << basePixmap;
        return false;
    }

    invalidate();
    _basePixmap = basePixmap;
    return true;
}

bool ScaledPixmap::isValid()
{
    if(!_pixmaps.at(DMHelper::PixmapSize_Full).isNull())
        return true;

    if(_basePixmap.isEmpty())
        return false;

    return _pixmaps[DMHelper::PixmapSize_Full].load(_basePixmap);
}

void ScaledPixmap::invalidate()
{
    for(int i = 0; i < DMHelper::PixmapSize_Count; ++i)
        _pixmaps[i] = QPixmap();
}

QPixmap ScaledPixmap::getPixmap(DMHelper::PixmapSize pixmapSize)
{
    if(!isValid())
        return QPixmap();

    if(_pixmaps.at(pixmapSize).isNull())
    {
        _pixmaps[pixmapSize] = _pixmaps.at(DMHelper::PixmapSize_Full).scaled(DMHelper::PixmapSizes[pixmapSize][0],
                                                                             DMHelper::PixmapSizes[pixmapSize][1],
                                                                             Qt::KeepAspectRatio,
                                                                             Qt::SmoothTransformation);
    }

    return _pixmaps.at(pixmapSize);
}
