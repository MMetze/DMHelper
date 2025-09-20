#include "layerfowsettings.h"
#include "ui_layerfowsettings.h"
#include "colorpushbutton.h"
#include <QFileDialog>
#include <QImageReader>
#include <QPainter>
#include <QMessageBox>

LayerFowSettings::LayerFowSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerFowSettings),
    _texturePixmap()
{
    ui->setupUi(this);

    connect(ui->btnColor, &ColorPushButton::colorChanged, this, &LayerFowSettings::handleColorChanged);
    connect(ui->edtTextureFile, &QLineEdit::editingFinished, this, &LayerFowSettings::handleTextureFileChanged);
    connect(ui->btnBrowseTexture, &QPushButton::clicked, this, &LayerFowSettings::handleTextureBrowseClicked);
    connect(ui->sliderScale, &QSlider::valueChanged, this, &LayerFowSettings::handleScaleChanged);
    connect(ui->chkTransparent, &QCheckBox::clicked, this, &LayerFowSettings::updatePreview);

    ui->lblPreview->setAutoFillBackground(true);
    ui->btnColor->setRotationVisible(false);

    updatePreview();
}

LayerFowSettings::~LayerFowSettings()
{
    delete ui;
}

QColor LayerFowSettings::fowColor() const
{
    QColor result = ui->btnColor->getColor();
    if(ui->chkTransparent->isChecked())
        result.setAlpha(0);
    return result;
}

QString LayerFowSettings::fowTextureFile() const
{
    return ui->edtTextureFile->text();
}

int LayerFowSettings::fowScale() const
{
    return ui->sliderScale->value();
}

void LayerFowSettings::setFowColor(const QColor& color)
{
    ui->btnColor->setColor(QColor(color.red(), color.green(), color.blue()));
    ui->chkTransparent->setChecked(color.alpha() == 0);
    updatePreview();
}

void LayerFowSettings::setFowTextureFile(const QString& textureFile)
{
    ui->edtTextureFile->setText(textureFile);
    _texturePixmap = QPixmap();
    updatePreview();
}

void LayerFowSettings::setFowScale(int scale)
{
    if((scale < ui->sliderScale->minimum()) || (scale > ui->sliderScale->maximum()))
        return;

    ui->sliderScale->setValue(scale);
    updatePreview();
}

bool LayerFowSettings::eventFilter(QObject *o, QEvent *e)
{
    if(o == ui->framePreview)
    {
        if(e->type() == QEvent::Resize)
        {
            QSize size = ui->framePreview->size();
            QMargins margins = ui->framePreview->contentsMargins();
            int sideLength = qMin(size.width() - margins.left() - margins.right() - (ui->lblPreview->lineWidth() * 2) - 12,
                                  size.height() - margins.top() - margins.bottom() - (ui->lblPreview->lineWidth() * 2) - 12);
            ui->lblPreview->setFixedSize(sideLength, sideLength);
        }
    }

    return QDialog::eventFilter(o, e);
}

void LayerFowSettings::handleColorChanged(const QColor& color)
{
    updatePreview();
    emit fowColorChanged(color);
}

void LayerFowSettings::handleTextureFileChanged()
{
    _texturePixmap = QPixmap();
    updatePreview();
    emit fowTextureFileChanged(ui->edtTextureFile->text());
}

void LayerFowSettings::handleTextureBrowseClicked()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, QString("Select Texture Image..."));
    if((filename.isEmpty()) || (!QImageReader(filename).canRead()))
        return;

    QImage newImage(filename);
    if(newImage.isNull())
    {
        QMessageBox::critical(nullptr, QString("Error"), QString("Unable to load texture image from: ") + filename);
        qDebug() << "[LayerFowSettings] ERROR: Unable to load texture image from: " << filename;
        return;
    }

    ui->edtTextureFile->setText(filename);
    handleTextureFileChanged();
}

void LayerFowSettings::handleScaleChanged(int scale)
{
    updatePreview();
    emit fowScaleChanged(scale);
}

void LayerFowSettings::updatePreview()
{
    if(ui->edtTextureFile->text().isEmpty())
    {
        QPixmap colorPixmap(ui->lblPreview->size());
        colorPixmap.fill(fowColor());
        ui->lblPreview->setPixmap(colorPixmap);
    }
    else
    {
        if(_texturePixmap.isNull())
            _texturePixmap = QPixmap(ui->edtTextureFile->text()).scaled(ui->lblPreview->size());

        if(_texturePixmap.isNull())
        {
            QMessageBox::critical(nullptr, QString("Error"), QString("Unable to load texture image from: ") + ui->edtTextureFile->text());
            qDebug() << "[LayerFowSettings] ERROR: Unable to load texture image from: " << ui->edtTextureFile->text();
            return;
        }

        // Draw a scaled and tiled version of the texture
        QPixmap pixmap = _texturePixmap;
        pixmap.fill(fowColor());
        QPixmap scaledPixmap = _texturePixmap.scaled(_texturePixmap.size() * ui->sliderScale->value() / 100);
        QPainter p(&pixmap);
            for(int x = 0; x < pixmap.width(); x += scaledPixmap.width())
                for(int y = 0; y < pixmap.height(); y += scaledPixmap.height())
                    p.drawPixmap(x, y, scaledPixmap);
        p.end();

        ui->lblPreview->setPixmap(pixmap);
    }
}
