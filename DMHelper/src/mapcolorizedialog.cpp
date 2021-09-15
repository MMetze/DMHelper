#include "mapcolorizedialog.h"
#include "ui_mapcolorizedialog.h"
#include <QDoubleValidator>
#include <QPainter>

MapColorizeDialog::MapColorizeDialog(QImage originalImage, const MapColorizeFilter& filter, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapColorizeDialog),
    _originalImage(originalImage),
    _scaledOriginal(),
    _filter(filter)
{
    ui->setupUi(this);

    _originalImage = originalImage;
    if(_originalImage.format() != QImage::Format_ARGB32_Premultiplied)
        _originalImage.convertTo(QImage::Format_ARGB32_Premultiplied);

    readFilter();
    applyFilter();

    connect(ui->btnApply, &QAbstractButton::clicked, this, &MapColorizeDialog::applyFilter);
    connect(ui->btnReset, &QAbstractButton::clicked, this, &MapColorizeDialog::resetFilter);

    ui->cmbPresets->addItem(QString("No Preset"));
    ui->cmbPresets->addItem(QString("Sepia"));
    ui->cmbPresets->addItem(QString("Greyscale"));
    ui->cmbPresets->addItem(QString("Darkness"));
    ui->cmbPresets->addItem(QString("Morning"));
    connect(ui->cmbPresets, &QComboBox::currentTextChanged, this, &MapColorizeDialog::presetSelected);

    ui->edt0x0->setValidator(new QDoubleValidator());
    ui->edt0x1->setValidator(new QDoubleValidator());
    ui->edt0x2->setValidator(new QDoubleValidator());
    ui->edt1x0->setValidator(new QDoubleValidator());
    ui->edt1x1->setValidator(new QDoubleValidator());
    ui->edt1x2->setValidator(new QDoubleValidator());
    ui->edt2x0->setValidator(new QDoubleValidator());
    ui->edt2x1->setValidator(new QDoubleValidator());
    ui->edt2x2->setValidator(new QDoubleValidator());

    ui->btnColor->setRotationVisible(false);

    connectControls(true);
}

MapColorizeDialog::~MapColorizeDialog()
{
    delete ui;
}

MapColorizeFilter MapColorizeDialog::getFilter() const
{
    return _filter;
}

void MapColorizeDialog::applyFilter()
{
    checkScaledOriginal();

    _filter._r2r = ui->edt0x0->text().toDouble();
    _filter._g2r = ui->edt0x1->text().toDouble();
    _filter._b2r = ui->edt0x2->text().toDouble();
    _filter._r2g = ui->edt1x0->text().toDouble();
    _filter._g2g = ui->edt1x1->text().toDouble();
    _filter._b2g = ui->edt1x2->text().toDouble();
    _filter._r2b = ui->edt2x0->text().toDouble();
    _filter._g2b = ui->edt2x1->text().toDouble();
    _filter._b2b = ui->edt2x2->text().toDouble();

    _filter._sr = static_cast<qreal>(ui->slideR->value()) / 100.0;
    _filter._sg = static_cast<qreal>(ui->slideG->value()) / 100.0;
    _filter._sb = static_cast<qreal>(ui->slideB->value()) / 100.0;

    _filter._isOverlay = ui->grpOverlay->isChecked();
    _filter._overlayColor = ui->btnColor->getColor();
    _filter._overlayAlpha = ui->slideOpacity->value();

    ui->lblColored->setPixmap(QPixmap::fromImage(_filter.apply(_scaledOriginal)));
}

void MapColorizeDialog::resetFilter()
{
    connectControls(false);

    ui->lblOriginal->setPixmap(QPixmap::fromImage(_scaledOriginal));
    ui->lblColored->setPixmap(QPixmap::fromImage(_scaledOriginal));
    _filter.reset();
    readFilter();
    ui->cmbPresets->setCurrentIndex(0);
    applyFilter();

    connectControls(true);
}

void MapColorizeDialog::presetSelected(const QString &text)
{
    if(text == QString("Sepia"))
    {
        ui->edt0x0->setText(QString("0.39"));
        ui->edt0x1->setText(QString("0.769"));
        ui->edt0x2->setText(QString("0.189"));
        ui->edt1x0->setText(QString("0.349"));
        ui->edt1x1->setText(QString("0.686"));
        ui->edt1x2->setText(QString("0.168"));
        ui->edt2x0->setText(QString("0.272"));
        ui->edt2x1->setText(QString("0.534"));
        ui->edt2x2->setText(QString("0.131"));
    }
    else if(text == QString("Greyscale"))
    {
        ui->edt0x0->setText(QString("0.33"));
        ui->edt0x1->setText(QString("0.33"));
        ui->edt0x2->setText(QString("0.33"));
        ui->edt1x0->setText(QString("0.33"));
        ui->edt1x1->setText(QString("0.33"));
        ui->edt1x2->setText(QString("0.33"));
        ui->edt2x0->setText(QString("0.33"));
        ui->edt2x1->setText(QString("0.33"));
        ui->edt2x2->setText(QString("0.33"));
    }
    else if(text == QString("Darkness"))
    {
        ui->edt0x0->setText(QString("0.4"));
        ui->edt0x1->setText(QString("0.0"));
        ui->edt0x2->setText(QString("0.0"));
        ui->edt1x0->setText(QString("0.0"));
        ui->edt1x1->setText(QString("0.4"));
        ui->edt1x2->setText(QString("0.0"));
        ui->edt2x0->setText(QString("0.2"));
        ui->edt2x1->setText(QString("0.2"));
        ui->edt2x2->setText(QString("0.65"));
    }
    else if(text == QString("Morning"))
    {
        ui->edt0x0->setText(QString("1.33"));
        ui->edt0x1->setText(QString("0.0"));
        ui->edt0x2->setText(QString("0.0"));
        ui->edt1x0->setText(QString("0.0"));
        ui->edt1x1->setText(QString("1.33"));
        ui->edt1x2->setText(QString("0.0"));
        ui->edt2x0->setText(QString("0.0"));
        ui->edt2x1->setText(QString("0.0"));
        ui->edt2x2->setText(QString("0.66"));
    }

    applyFilter();
}

void MapColorizeDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    applyFilter();
}

void MapColorizeDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    checkScaledOriginal();
}

void MapColorizeDialog::connectControls(bool connectMe)
{
    if(connectMe)
    {
        connect(ui->edt0x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt0x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt0x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt1x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt1x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt1x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt2x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt2x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->edt2x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        connect(ui->slideR, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        connect(ui->slideG, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        connect(ui->slideB, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        connect(ui->grpOverlay, &QGroupBox::clicked, this, &MapColorizeDialog::applyFilter);
        connect(ui->btnColor, &ColorPushButton::colorChanged, this, &MapColorizeDialog::applyFilter);
        connect(ui->slideOpacity, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
    }
    else
    {
        disconnect(ui->edt0x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt0x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt0x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt1x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt1x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt1x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt2x0, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt2x1, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->edt2x2, &QLineEdit::editingFinished, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->slideR, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->slideG, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->slideB, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->grpOverlay, &QGroupBox::clicked, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->btnColor, &ColorPushButton::colorChanged, this, &MapColorizeDialog::applyFilter);
        disconnect(ui->slideOpacity, &QSlider::valueChanged, this, &MapColorizeDialog::applyFilter);
    }
}

void MapColorizeDialog::checkScaledOriginal()
{
    if(_scaledOriginal.size() != ui->lblOriginal->size())
    {
        _scaledOriginal = _originalImage.scaled(ui->lblOriginal->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->lblOriginal->setPixmap(QPixmap::fromImage(_scaledOriginal));
        if(ui->lblColored->pixmap(Qt::ReturnByValue).isNull())
            ui->lblColored->setPixmap(ui->lblOriginal->pixmap(Qt::ReturnByValue));
    }
}

void MapColorizeDialog::readFilter()
{
    ui->edt0x0->setText(QString::number(_filter._r2r, 'f', 3));
    ui->edt0x1->setText(QString::number(_filter._g2r, 'f', 3));
    ui->edt0x2->setText(QString::number(_filter._b2r, 'f', 3));
    ui->edt1x0->setText(QString::number(_filter._r2g, 'f', 3));
    ui->edt1x1->setText(QString::number(_filter._g2g, 'f', 3));
    ui->edt1x2->setText(QString::number(_filter._b2g, 'f', 3));
    ui->edt2x0->setText(QString::number(_filter._r2b, 'f', 3));
    ui->edt2x1->setText(QString::number(_filter._g2b, 'f', 3));
    ui->edt2x2->setText(QString::number(_filter._b2b, 'f', 3));

    ui->slideR->setValue(static_cast<int>(_filter._sr * 100.0));
    ui->slideG->setValue(static_cast<int>(_filter._sg * 100.0));
    ui->slideB->setValue(static_cast<int>(_filter._sb * 100.0));

    ui->grpOverlay->setChecked(_filter._isOverlay);
    ui->btnColor->setColor(_filter._overlayColor);
    ui->slideOpacity->setValue(_filter._overlayAlpha);
}
