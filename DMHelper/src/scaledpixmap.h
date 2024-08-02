#ifndef SCALEDPIXMAP_H
#define SCALEDPIXMAP_H

#include <QVector>
#include <QPixmap>
#include "dmconstants.h"

namespace DMHelper
{
    enum PixmapSize
    {
        PixmapSize_Full = 0,
        PixmapSize_Showcase,
        PixmapSize_Battle,
        PixmapSize_Animate,
        PixmapSize_Thumb,

        PixmapSize_Count
    } ;

    const int PixmapSizes[PixmapSize_Count][2] = {
        { -1,  -1}, // PixmapSize_Full = fullsize,
        {250, 350}, // PixmapSize_Showcase = large-scale
        {250, 250}, // PixmapSize_Battle = mid-size square
        {110, 150}, // PixmapSize_Animate = mid-sized icon for animation dialogs
        {CHARACTER_ICON_WIDTH, CHARACTER_ICON_HEIGHT}  // PixmapSize_Thumb = small thumbnail
    };
}

class ScaledPixmap
{
public:
    ScaledPixmap();
    explicit ScaledPixmap(const ScaledPixmap &obj);  // copy constructor
    virtual ~ScaledPixmap();

    static ScaledPixmap* defaultPixmap();
    static void cleanupDefaultPixmap();

    bool setBasePixmap(const QString& basePixmap);
    bool isValid() const;
    void invalidate();

    QPixmap getPixmap(DMHelper::PixmapSize pixmapSize);

protected:
    QVector<QPixmap> _pixmaps;
    QString _basePixmap;

private:
    static ScaledPixmap* _defaultPixmap;

};

#endif // SCALEDPIXMAP_H
