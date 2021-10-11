#ifndef MAPCOLORIZEFILTER_H
#define MAPCOLORIZEFILTER_H

#include <QColor>
#include <QImage>

class MapColorizeFilter
{
public:
    MapColorizeFilter();
    virtual ~MapColorizeFilter();

    QImage apply(const QImage& in);
    bool isValid();
    void reset();

    qreal _r2r;
    qreal _g2r;
    qreal _b2r;

    qreal _r2g;
    qreal _g2g;
    qreal _b2g;

    qreal _r2b;
    qreal _g2b;
    qreal _b2b;

    qreal _sr;
    qreal _sg;
    qreal _sb;

    bool _isOverlay;
    QColor _overlayColor;
    int _overlayAlpha;
};

#endif // MAPCOLORIZEFILTER_H
