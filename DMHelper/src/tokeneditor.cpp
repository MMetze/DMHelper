#include "tokeneditor.h"
#include "optionscontainer.h"
#include <QPainter>
#include <QDebug>

TokenEditor::TokenEditor(const QString& sourceFile, bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, QObject *parent) :
    QObject{parent},
    _backgroundFill{backgroundFill},
    _backgroundFillColor{backgroundFillColor},
    _transparent{transparent ? DMHelper::TransparentType_TransparentColor : DMHelper::TransparentType_None},
    _transparentColor{transparentColor.rgb()},
    _transparentLevel{transparentLevel},
    _colorize{false},
    _colorizeColor{},
    _maskApplied{maskApplied},
    _maskFile{},
    _maskImage{},
    _scaledMaskImage{},
    _frameApplied{frameApplied},
    _frameFile{},
    _frameImage{},
    _scaledFrameImage{},
    _zoom{zoom},
    _offset{offset},
    _sourceFile(),
    _sourceImage(),
    _finalImage(),
    _dirty(true)
{
    setSourceFile(sourceFile);

    if(!maskFile.isNull())
        setMaskFile(maskFile);

    if(!frameFile.isNull())
        setFrameFile(frameFile);
}

TokenEditor::TokenEditor(QImage sourceImage, bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, QObject *parent) :
    QObject{parent},
    _backgroundFill{backgroundFill},
    _backgroundFillColor{backgroundFillColor},
    _transparent{transparent ? DMHelper::TransparentType_TransparentColor : DMHelper::TransparentType_None},
    _transparentColor{transparentColor.rgb()},
    _transparentLevel{transparentLevel},
    _maskApplied{maskApplied},
    _maskFile{maskFile},
    _maskImage{},
    _scaledMaskImage{},
    _frameApplied{frameApplied},
    _frameFile{frameFile},
    _frameImage{},
    _scaledFrameImage{},
    _zoom{zoom},
    _offset{offset},
    _sourceFile(),
    _sourceImage(),
    _finalImage(),
    _dirty(true)
{
    setSourceImage(sourceImage);

    if(!maskFile.isNull())
        setMaskFile(maskFile);

    if(!frameFile.isNull())
        setFrameFile(frameFile);
}

QImage TokenEditor::getFinalImage()
{
    if(_dirty)
        updateFinalImage();

    return _finalImage;
}

bool TokenEditor::isBackgroundFill() const
{
    return _backgroundFill;
}

QColor TokenEditor::getBackgroundFillColor() const
{
    return _backgroundFillColor;
}

bool TokenEditor::isTransparent() const
{
    return _transparent == DMHelper::TransparentType_TransparentColor;
}

DMHelper::TransparentType TokenEditor::getTransparent() const
{
    return _transparent;
}

QColor TokenEditor::getTransparentColor() const
{
    return QColor(_transparentColor);
}

int TokenEditor::getTransparentLevel() const
{
    return _transparentLevel;
}

bool TokenEditor::isColorize() const
{
    return _colorize;
}

QColor TokenEditor::getColorizeColor() const
{
    return _colorizeColor;
}

bool TokenEditor::isMaskApplied() const
{
    return _maskApplied;
}

QString TokenEditor::getMaskFile() const
{
    return _maskFile;
}

QImage TokenEditor::getMaskImage() const
{
    return _maskImage;
}

bool TokenEditor::isFrameApplied() const
{
    return _frameApplied;
}

QString TokenEditor::getFrameFile() const
{
    return _frameFile;
}

QImage TokenEditor::getFrameImage() const
{
    return _frameImage;
}

qreal TokenEditor::getZoom() const
{
    return _zoom;
}

QPoint TokenEditor::getOffset() const
{
    return _offset;
}

void TokenEditor::applyOptionsToEditor(const OptionsContainer& options)
{
    _dirty = true;

    setBackgroundFill(options.getTokenBackgroundFill());
    setBackgroundFillColor(options.getTokenBackgroundFillColor());
    setTransparent(options.getTokenTransparent());
    setTransparentColor(options.getTokenTransparentColor());
    setTransparentLevel(options.getTokenTransparentLevel());
    setMaskApplied(options.getTokenMaskApplied());
    setMaskFile(options.getTokenMaskFile());
    setFrameApplied(options.getTokenFrameApplied());
    setFrameFile(options.getTokenFrameFile());

    emit imageDirty();
}

void TokenEditor::applyEditorToOptions(OptionsContainer& options)
{
    options.setTokenBackgroundFill(isBackgroundFill());
    options.setTokenBackgroundFillColor(getBackgroundFillColor());
    options.setTokenTransparent(isTransparent());
    options.setTokenTransparentColor(getTransparentColor());
    options.setTokenTransparentLevel(getTransparentLevel());
    options.setTokenMaskApplied(isMaskApplied());
    options.setTokenMaskFile(getMaskFile());
    options.setTokenFrameApplied(isFrameApplied());
    options.setTokenFrameFile(getFrameFile());
}

void TokenEditor::setSourceFile(const QString& sourceFile)
{
    if((sourceFile.isEmpty()) || (sourceFile == _sourceFile))
        return;

    _sourceFile = sourceFile;

    QImage newSourceImage(_sourceFile);
    if(newSourceImage.isNull())
        qDebug() << "[TokenEditor] ERROR: Unabled to load source image from: " << _sourceFile << ". Setting source image to null.";

    setSourceImage(newSourceImage);
}

void TokenEditor::setSourceImage(const QImage& sourceImage)
{
    if(sourceImage == _sourceImage)
        return;

    _sourceImage = sourceImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    int maxDim = qMax(_sourceImage.width(), _sourceImage.height());
    _finalImage = QImage(QSize(maxDim, maxDim), QImage::Format_ARGB32);
    rescaleImages();

    setDirty();
}

void TokenEditor::setBackgroundFill(bool backgroundFill)
{
    if(backgroundFill == _backgroundFill)
        return;

    _backgroundFill = backgroundFill;
    setDirty();
}

void TokenEditor::setBackgroundFillColor(const QColor& color)
{
    if(color == _backgroundFillColor)
        return;

    _backgroundFillColor = color;
    setDirty();
}

void TokenEditor::setTransparent(bool transparent)
{
    if(((transparent) && (_transparent == DMHelper::TransparentType_TransparentColor)) ||
       ((!transparent) && (_transparent == DMHelper::TransparentType_None)))
        return;

    if(transparent)
        _transparent = DMHelper::TransparentType_TransparentColor;
    else
        _transparent = DMHelper::TransparentType_None;

    setDirty();
}

void TokenEditor::setTransparentValue(DMHelper::TransparentType transparent)
{
    if(transparent == _transparent)
        return;

    _transparent = transparent;
    setDirty();
}

void TokenEditor::setTransparentColor(const QColor& transparentColor)
{
    if(transparentColor.toRgb() == _transparentColor)
        return;

    _transparentColor = transparentColor.rgb();
    setDirty();
}

void TokenEditor::setTransparentLevel(int transparentLevel)
{
    if(transparentLevel == _transparentLevel)
        return;

    _transparentLevel = transparentLevel;
    setDirty();
}

void TokenEditor::setColorize(bool colorize)
{
    if(colorize == _colorize)
        return;

    _colorize = colorize;
    setDirty();
}

void TokenEditor::setColorizeColor(const QColor& colorizeColor)
{
    if(colorizeColor == _colorizeColor)
        return;

    _colorizeColor = colorizeColor;
    setDirty();
}

void TokenEditor::setMaskApplied(bool maskApplied)
{
    if(maskApplied == _maskApplied)
        return;

    _maskApplied = maskApplied;
    setDirty();
}

void TokenEditor::setMaskFile(const QString& maskFile)
{
    if(maskFile == _maskFile)
        return;

    _maskFile = maskFile;
    QImage newMaskImage(_maskFile);
    if(newMaskImage.isNull())
        qDebug() << "[TokenEditor] ERROR: Unable to load mask image from: " << _maskFile << ". Setting mask image to null.";

    setMaskImage(newMaskImage);
}

void TokenEditor::setMaskImage(QImage maskImage)
{
    if(maskImage == _maskImage)
        return;

    _maskImage = maskImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    _scaledMaskImage = _maskImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);

    setDirty();
}

void TokenEditor::setFrameApplied(bool frameApplied)
{
    if(frameApplied == _frameApplied)
        return;

    _frameApplied = frameApplied;
    setDirty();
}

void TokenEditor::setFrameFile(const QString& frameFile)
{
    if(frameFile == _frameFile)
        return;

    _frameFile = frameFile;
    QImage newFrameImage(_frameFile);
    if(newFrameImage.isNull())
        qDebug() << "[TokenEditor] ERROR: Unable to load frame image from: " << _frameFile << ". Setting frame image to null.";

    setFrameImage(newFrameImage);
}

void TokenEditor::setFrameImage(QImage frameImage)
{
    if(frameImage == _frameImage)
        return;

    _frameImage = frameImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    _scaledFrameImage = _frameImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);

    setDirty();
}

void TokenEditor::setZoom(qreal zoom)
{
    if(zoom == _zoom)
        return;

    _zoom = zoom;
    setDirty();
}

void TokenEditor::zoomIn()
{
    _zoom *= 1.1;
    setDirty();
}

void TokenEditor::zoomOut()
{
    _zoom /= 1.1;
    setDirty();
}

void TokenEditor::setOffset(const QPoint& offset)
{
    if(offset == _offset)
        return;

    _offset = offset;
    setDirty();
}

void TokenEditor::moveOffset(const QPoint& delta)
{
    _offset += delta;
    setDirty();
}

void TokenEditor::updateFinalImage()
{
    if((_sourceImage.isNull()) || (_finalImage.isNull()))
        return;

    QSize zoomedSize = (_sourceImage.size().toSizeF() * _zoom).toSize();
    QImage scaledSource = _sourceImage.scaled(zoomedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    _finalImage.fill(Qt::transparent);
    int xOffset = _offset.x() + (_finalImage.width() - scaledSource.width()) / 2;
    int yOffset = _offset.y() + (_finalImage.height() - scaledSource.height()) / 2;

    QImage interimSource(_finalImage.size(), _sourceImage.format());
    interimSource.fill(_backgroundFill ? _backgroundFillColor : Qt::transparent);

    if(_transparent == DMHelper::TransparentType_TransparentColor)
    {
        int yStart = qMax(0, -yOffset);
        int yEnd = qMin(scaledSource.height(), _finalImage.height() - yOffset);
        int xStart = qMax(0, -xOffset);
        int xEnd = qMin(scaledSource.width(), _finalImage.width() - xOffset);

        for (int y = yStart; y < yEnd; y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(scaledSource.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(interimSource.scanLine(y + yOffset));
            for(int x = xStart; x < xEnd; x++)
            {
                if(!fuzzyColorMatch(inputLine[x], _transparentColor))
                    outputLine[x + xOffset] = inputLine[x];
            }
        }
    }
    else if(_transparent == DMHelper::TransparentType_RedChannel)
    {
        int yStart = qMax(0, -yOffset);
        int yEnd = qMin(scaledSource.height(), _finalImage.height() - yOffset);
        int xStart = qMax(0, -xOffset);
        int xEnd = qMin(scaledSource.width(), _finalImage.width() - xOffset);

        for (int y = yStart; y < yEnd; y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(scaledSource.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(interimSource.scanLine(y + yOffset));
            for(int x = xStart; x < xEnd; x++)
            {
                const QRgb &rgb = inputLine[x];
                outputLine[x + xOffset] = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), qRed(rgb));
            }
        }
    }
    else if(_transparent == DMHelper::TransparentType_GreenChannel)
    {
        int yStart = qMax(0, -yOffset);
        int yEnd = qMin(scaledSource.height(), _finalImage.height() - yOffset);
        int xStart = qMax(0, -xOffset);
        int xEnd = qMin(scaledSource.width(), _finalImage.width() - xOffset);

        for (int y = yStart; y < yEnd; y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(scaledSource.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(interimSource.scanLine(y + yOffset));
            for(int x = xStart; x < xEnd; x++)
            {
                const QRgb &rgb = inputLine[x];
                outputLine[x + xOffset] = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), qGreen(rgb));
            }
        }
    }
    else if(_transparent == DMHelper::TransparentType_BlueChannel)
    {
        int yStart = qMax(0, -yOffset);
        int yEnd = qMin(scaledSource.height(), _finalImage.height() - yOffset);
        int xStart = qMax(0, -xOffset);
        int xEnd = qMin(scaledSource.width(), _finalImage.width() - xOffset);

        for (int y = yStart; y < yEnd; y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(scaledSource.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(interimSource.scanLine(y + yOffset));
            for(int x = xStart; x < xEnd; x++)
            {
                const QRgb &rgb = inputLine[x];
                outputLine[x + xOffset] = qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), qBlue(rgb));
            }
        }
    }
    else
    {
        QPainter p(&interimSource);
        p.drawImage(xOffset, yOffset, scaledSource);
    }

    if(_maskApplied)
    {
        QPainter p(&interimSource);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawImage((_finalImage.width() - _scaledMaskImage.width())/2, (_finalImage.height() - _scaledMaskImage.height()) / 2, _scaledMaskImage);
    }

    QPainter pFinal(&_finalImage);
    pFinal.drawImage((_finalImage.width() - interimSource.width())/2, (_finalImage.height() - interimSource.height()) / 2, interimSource);
    if(_frameApplied)
        pFinal.drawImage((_finalImage.width() - _scaledFrameImage.width())/2, (_finalImage.height() - _scaledFrameImage.height()) / 2, _scaledFrameImage);

    _dirty = false;
}

void TokenEditor::rescaleImages()
{
    if(!_maskImage.isNull())
        _scaledMaskImage = _maskImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);

    if(!_frameImage.isNull())
    _scaledFrameImage = _frameImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);
}

void TokenEditor::setDirty()
{
    if(_dirty)
        return;

    _dirty = true;
    emit imageDirty();
}

bool TokenEditor::fuzzyColorMatch(QRgb first, QRgb second)
{
    return ((qAbs(qRed(first) - qRed(second)) <= _transparentLevel) &&
            (qAbs(qGreen(first) - qGreen(second)) <= _transparentLevel) &&
            (qAbs(qBlue(first) - qBlue(second)) <= _transparentLevel));
}
