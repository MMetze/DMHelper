#ifndef MAPCOLORIZEFILTER_H
#define MAPCOLORIZEFILTER_H

#include <QColor>
#include <QImage>

class MapColorizeFilter
{
public:
    MapColorizeFilter();
    virtual ~MapColorizeFilter();

    QImage apply(const QImage& in) const;
    bool isValid() const;
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

inline bool operator==(const MapColorizeFilter& lhs, const MapColorizeFilter& rhs)
{
    return ((lhs._r2r == rhs._r2r) && (lhs._g2r == rhs._g2r) && (lhs._b2r == rhs._b2r) &&
            (lhs._r2g == rhs._r2g) && (lhs._g2g == rhs._g2g) && (lhs._b2g == rhs._b2g) &&
            (lhs._r2b == rhs._r2b) && (lhs._g2b == rhs._g2b) && (lhs._b2b == rhs._b2b) &&
            (lhs._isOverlay == rhs._isOverlay) &&
            (lhs._overlayColor == rhs._overlayColor) &&
            (lhs._overlayAlpha == rhs._overlayAlpha));
}

inline bool operator!=(const MapColorizeFilter& lhs, const MapColorizeFilter& rhs)
{
    return !operator==(lhs,rhs);
}

#endif // MAPCOLORIZEFILTER_H
