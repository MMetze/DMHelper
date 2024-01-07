#ifndef TOKENEDITOR_H
#define TOKENEDITOR_H

#include <QObject>
#include <QImage>
#include <QPoint>
#include "dmconstants.h"

class OptionsContainer;

class TokenEditor : public QObject
{
    Q_OBJECT
public:
    static const int TRANSPARENT_LEVEL_DEFAULT = 15;

    explicit TokenEditor(const QString& sourceFile = QString(), bool backgroundFill = false, const QColor& backgroundFillColor = Qt::white, bool transparent = false, const QColor& transparentColor = Qt::white, int transparentLevel = TRANSPARENT_LEVEL_DEFAULT, bool maskApplied = false, const QString& maskFile = QString(), bool frameApplied = false, const QString& frameFile = QString(), qreal zoom = 1.0, const QPoint& offset = QPoint(), QObject *parent = nullptr);
    explicit TokenEditor(QImage sourceImage, bool backgroundFill = false, const QColor& backgroundFillColor = Qt::white, bool transparent = false, const QColor& transparentColor = Qt::white, int transparentLevel = TRANSPARENT_LEVEL_DEFAULT, bool maskApplied = false, const QString& maskFile = QString(), bool frameApplied = false, const QString& frameFile = QString(), qreal zoom = 1.0, const QPoint& offset = QPoint(), QObject *parent = nullptr);

    QImage getFinalImage();

    bool isBackgroundFill() const;
    QColor getBackgroundFillColor() const;

    bool isTransparent() const;
    DMHelper::TransparentType getTransparent() const;
    QColor getTransparentColor() const;
    int getTransparentLevel() const;

    bool isColorize() const;
    QColor getColorizeColor() const;

    bool isMaskApplied() const;
    QString getMaskFile() const;
    QImage getMaskImage() const;

    bool isFrameApplied() const;
    QString getFrameFile() const;
    QImage getFrameImage() const;

    qreal getZoom() const;
    QPoint getOffset() const;

    bool isSquareFinalImage() const;

    void applyOptionsToEditor(const OptionsContainer& options);
    void applyEditorToOptions(OptionsContainer& options);

public slots:
    void setSourceFile(const QString& sourceFile);
    void setSourceImage(const QImage& sourceImage);

    void setBackgroundFill(bool backgroundFill);
    void setBackgroundFillColor(const QColor& color);

    void setTransparent(bool transparent);
    void setTransparentValue(DMHelper::TransparentType transparent);
    void setTransparentColor(const QColor& transparentColor);
    void setTransparentLevel(int transparentLevel);

    void setColorize(bool colorize);
    void setColorizeColor(const QColor& colorizeColor);

    void setMaskApplied(bool maskApplied);
    void setMaskFile(const QString& maskFile);
    void setMaskImage(QImage maskImage);

    void setFrameApplied(bool frameApplied);
    void setFrameFile(const QString& frameFile);
    void setFrameImage(QImage frameImage);

    void setZoom(qreal zoom);
    void zoomIn();
    void zoomOut();

    void setOffset(const QPoint& offset);
    void moveOffset(const QPoint& delta);

    void setSquareFinalImage(bool squareFinalImage);

signals:
    void imageDirty();

protected:
    void updateFinalImage();
    void resizeFinalImage();
    void rescaleImages();
    void setDirty();
    bool fuzzyColorMatch(QRgb first, QRgb second);

    // The copy functions
    void copyTransparentColor(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyRedChannel(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyGreenChannel(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyBlueChannel(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyTransparentColorColorize(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyRedChannelColorize(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyGreenChannelColorize(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyBlueChannelColorize(QImage& dest, const QImage& source, int xOffset, int yOffset);
    void copyColorize(QImage& dest, const QImage& source, int xOffset, int yOffset);

    bool _backgroundFill;
    QColor _backgroundFillColor;

    DMHelper::TransparentType _transparent;
    QRgb _transparentColor;
    int _transparentLevel;

    bool _colorize;
    QColor _colorizeColor;

    bool _maskApplied;
    QString _maskFile;
    QImage _maskImage;
    QImage _scaledMaskImage;

    bool _frameApplied;
    QString _frameFile;
    QImage _frameImage;
    QImage _scaledFrameImage;

    qreal _zoom;
    QPoint _offset;
    bool _squareFinalImage;

    QString _sourceFile;
    QImage _sourceImage;
    QImage _finalImage;

    bool _dirty;

};

#endif // TOKENEDITOR_H
