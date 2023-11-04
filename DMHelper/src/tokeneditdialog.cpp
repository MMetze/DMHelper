#include "tokeneditdialog.h"
#include "ui_tokeneditdialog.h"
#include <QPainter>

TokenEditDialog::TokenEditDialog(const QString& tokenFilename, TokenDetailMode mode, const QColor& background, int backgroundLevel, const QString& frameFile, const QString& maskFile, bool fill, QColor fillColor, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TokenEditDialog),
    _maskImage(),
    _frameImage(),
    _sourceImage(),
    _finalImage()
{
    ui->setupUi(this);

    _sourceImage.load(tokenFilename);
    if(_sourceImage.isNull())
    {
        qDebug() << "[TokenEditDialog] ERROR: Unabled to load source image from: " << tokenFilename;
    }
    else
    {
        int maxDim = qMax(_sourceImage.width(), _sourceImage.height());
        _finalImage = QImage(QSize(maxDim, maxDim), _sourceImage.format());
    }

    switch(mode)
    {
    case TokenDetailMode_TransparentColor:
        ui->btnTransparentColor->setChecked(true);
        break;
    case TokenDetailMode_FrameAndMask:
        ui->btnFrameAndMask->setChecked(true);
        break;
    case TokenDetailMode_Original:
    default:
        ui->btnOriginalImage->setChecked(true);
        break;
    };

    ui->btnColor->setColor(background);
    ui->btnColor->setRotationVisible(false);
    ui->sliderFuzzy->setValue(backgroundLevel);

    ui->edtFrameImage->setText(frameFile);
    ui->edtMaskImage->setText(maskFile);

    ui->chkFill->setChecked(fill);
    ui->btnFillColor->setColor(fillColor);

    connect(ui->btnOriginalImage, &QAbstractButton::toggled, this, &TokenEditDialog::updateImage);
    connect(ui->btnTransparentColor, &QAbstractButton::toggled, this, &TokenEditDialog::updateImage);
    connect(ui->btnColor, &ColorPushButton::colorChanged, this, &TokenEditDialog::updateImage);
    connect(ui->sliderFuzzy, &QAbstractSlider::valueChanged, this, &TokenEditDialog::updateImage);
    connect(ui->btnFrameAndMask, &QAbstractButton::toggled, this, &TokenEditDialog::updateImage);
    connect(ui->btnBrowseFrameImage, &QAbstractButton::clicked, this, &TokenEditDialog::browseFrame);
    connect(ui->btnBrowseMaskImage, &QAbstractButton::clicked, this, &TokenEditDialog::browseMask);
    connect(ui->edtFrameImage, &QLineEdit::textChanged, this, &TokenEditDialog::updateFrameMaskImages);
    connect(ui->edtMaskImage, &QLineEdit::textChanged, this, &TokenEditDialog::updateFrameMaskImages);

    updateFrameMaskImages();
}

TokenEditDialog::~TokenEditDialog()
{
    delete ui;
}

void TokenEditDialog::updateImage()
{
    _finalImage.fill(ui->chkFill->isChecked() ? ui->btnFillColor->getColor() : Qt::transparent);
    int xOffset = (_finalImage.width() - _sourceImage.width()) / 2;
    int yOffset = (_finalImage.height() - _sourceImage.height()) / 2;

    if(ui->btnOriginalImage->isChecked())
    {
        QPainter p(&_finalImage);
        p.drawImage(xOffset, yOffset, _sourceImage);
    }
    else if(ui->btnTransparentColor->isChecked())
    {
        QRgb transparentColor = ui->btnColor->getColor().rgb();

        for (int y = 0; y < _sourceImage.height(); y++)
        {
            const QRgb* inputLine = reinterpret_cast<const QRgb *>(_sourceImage.scanLine(y));
            QRgb* outputLine = reinterpret_cast<QRgb *>(_finalImage.scanLine(y + yOffset));
            for(int x = 0; x < _sourceImage.width(); x++)
            {
                if(!fuzzyColorMatch(inputLine[x], transparentColor))
                    outputLine[x + xOffset] = inputLine[x];
            }
        }
    }
    else if(ui->btnFrameAndMask->isChecked())
    {
        if((_frameImage.isNull()) || (_maskImage.isNull()))
        {
            qDebug() << "[TokenEditDialog] ERROR: Unable to load expected frame and masks, no image able to be set!";
            return;
        }

        QImage frameImage = _frameImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);
        QImage maskImage = _frameImage.scaled(_finalImage.size(), Qt::KeepAspectRatio);

        QImage interimSource = _sourceImage;
        QPainter pInt;
        pInt.begin(&interimSource);
            pInt.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            pInt.drawImage((_finalImage.width() - maskImage.width())/2, (_finalImage.height() - maskImage.height()) / 2, maskImage);
        pInt.end();

        QPainter p(&_finalImage);
        p.drawImage((_finalImage.width() - interimSource.width())/2, (_finalImage.height() - interimSource.height()) / 2, interimSource);
        p.drawImage((_finalImage.width() - frameImage.width())/2, (_finalImage.height() - frameImage.height()) / 2, frameImage);
    }
    else
    {
        qDebug() << "[TokenEditDialog] ERROR: Unexpected token state, no image able to be set!";
        return;
    }

    ui->lblToken->setPixmap(QPixmap::fromImage(_finalImage));
}

void TokenEditDialog::browseFrame()
{

}

void TokenEditDialog::browseMask()
{

}

void TokenEditDialog::updateFrameMaskImages()
{
    _frameImage = QImage(ui->edtFrameImage->text());
    if(_frameImage.isNull())
        qDebug() << "[TokenEditDialog] ERROR: Unable to load frame image from: " << ui->edtFrameImage->text();

    _maskImage = QImage(ui->edtMaskImage->text());
    if(_maskImage.isNull())
        qDebug() << "[TokenEditDialog] ERROR: Unable to load frame image from: " << ui->edtMaskImage->text();

    updateImage();
}

bool TokenEditDialog::fuzzyColorMatch(QRgb first, QRgb second)
{
    return ((qAbs(qRed(first) - qRed(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qGreen(first) - qGreen(second)) <= ui->sliderFuzzy->value()) &&
            (qAbs(qBlue(first) - qBlue(second)) <= ui->sliderFuzzy->value()));
}
