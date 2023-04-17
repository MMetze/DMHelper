#include "mapcolorizefilter.h"
#include <QPainter>
#include <QDebug>

MapColorizeFilter::MapColorizeFilter() :
    _r2r(1.0), _g2r(0.0), _b2r(0.0),
    _r2g(0.0), _g2g(1.0), _b2g(0.0),
    _r2b(0.0), _g2b(0.0), _b2b(1.0),
     _sr(1.0),  _sg(1.0),  _sb(1.0),
    _isOverlay(false),
    _overlayColor(0, 0, 0),
    _overlayAlpha(128)
{
}

MapColorizeFilter::~MapColorizeFilter()
{
}

QImage MapColorizeFilter::apply(const QImage& in) const
{
    if(!in.size().isValid())
        return QImage();

    if(in.depth() != 32)
    {
        qDebug() << "[MapColorizeFilter] ERROR: Filter cannot be applied because input image is not 32bpp!";
        return QImage();
    }

    QImage filteredImage(in.size(), in.format());

    for (int y = 0; y < in.height(); y++)
    {
        const QRgb* inputLine = reinterpret_cast<const QRgb *>(in.scanLine(y));
        QRgb* outputLine = reinterpret_cast<QRgb *>(filteredImage.scanLine(y));
        for (int x = 0; x < in.width(); x++)
        {
            qreal r = static_cast<qreal>(qRed(inputLine[x]));
            qreal g = static_cast<qreal>(qGreen(inputLine[x]));
            qreal b = static_cast<qreal>(qBlue(inputLine[x]));
            outputLine[x] = qRgba(qMin(qMax(static_cast<int>((_r2r*r + _g2r*g + _b2r*b) * _sr), 0), 255),
                                  qMin(qMax(static_cast<int>((_r2g*r + _g2g*g + _b2g*b) * _sg), 0), 255),
                                  qMin(qMax(static_cast<int>((_r2b*r + _g2b*g + _b2b*b) * _sb), 0), 255),
                                  qAlpha(inputLine[x]));
        }
    }

    if(_isOverlay)
    {
        QPainter painter(&filteredImage);
        QColor brushColor = _overlayColor;
        brushColor.setAlpha(_overlayAlpha);
        painter.setBrush(QBrush(brushColor));
        painter.drawRect(0, 0, filteredImage.width(), filteredImage.height());
    }

    return filteredImage;
}

bool MapColorizeFilter::isValid() const
{
    return((_r2r != 1.0) || (_g2r != 0.0) || (_b2r != 0.0) ||
           (_r2g != 0.0) || (_g2g != 1.0) || (_b2g != 0.0) ||
           (_r2b != 0.0) || (_g2b != 0.0) || (_b2b != 1.0) ||
           (_sr != 1.0) || (_sg != 1.0) || (_sb != 1.0) ||
           (_isOverlay));
}

void MapColorizeFilter::reset()
{
    _r2r = 1.0; _g2r = 0.0; _b2r = 0.0;
    _r2g = 0.0; _g2g = 1.0; _b2g = 0.0;
    _r2b = 0.0; _g2b = 0.0; _b2b = 1.0;
    _sr = 1.0;
    _sg = 1.0;
    _sb = 1.0;
    _isOverlay = false;
    _overlayColor = QColor(0, 0, 0);
    _overlayAlpha = 128;
}
