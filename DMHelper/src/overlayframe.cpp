#include "overlayframe.h"
#include "ui_overlayframe.h"
#include "overlay.h"
#include "overlaycounter.h"
#include "overlaytimer.h"
#include "dmconstants.h"
#include <QDebug>

OverlayFrame::OverlayFrame(Overlay* overlay, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::OverlayFrame),
    _overlay(overlay)
{
    ui->setupUi(this);

    if(!_overlay)
    {
        qDebug() << "[OverlayFrame] ERROR: null overlay pointer";
        return;
    }

    switch(_overlay->getOverlayType())
    {
        case DMHelper::OverlayType_Fear:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/hoodeyeless.png"));
            break;
        case DMHelper::OverlayType_Counter:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/icon_overlaycounter.png"));
            break;
        case DMHelper::OverlayType_Timer:
            ui->lblIcon->setPixmap(QPixmap(":/img/data/icon_overlaytimer.png"));
            break;
        default:
            qDebug() << "[OverlayFrame] ERROR: unknown overlay type: " << _overlay->getOverlayType();
            break;
    }

    ui->chkVisible->setChecked(_overlay->isVisible());
    ui->spinScale->setValue(_overlay->getScale());
    ui->sliderScale->setValue(static_cast<int>(_overlay->getScale() * 10.0));
    ui->spinOpacity->setValue(_overlay->getOpacity());
    ui->sliderOpacity->setValue(_overlay->getOpacity());

    connect(ui->chkVisible, &QCheckBox::toggled, _overlay, &Overlay::setVisible);
    connect(ui->sliderOpacity, &QSlider::valueChanged, this, &OverlayFrame::handleOpacityChanged);
    connect(ui->spinOpacity, qOverload<int>(&QSpinBox::valueChanged), this, &OverlayFrame::handleOpacityChanged);
    connect(ui->sliderScale, &QSlider::valueChanged, this, &OverlayFrame::handleScaleSliderChanged);
    connect(ui->spinScale, qOverload<qreal>(&QDoubleSpinBox::valueChanged), this, &OverlayFrame::handleScaleSpinChanged);

    int overlayType = _overlay->getOverlayType();
    ui->btnIncrease->setEnabled(overlayType == DMHelper::OverlayType_Counter);
    ui->btnDecrease->setEnabled(overlayType == DMHelper::OverlayType_Counter);
    if(overlayType == DMHelper::OverlayType_Counter)
    {
        connect(ui->btnIncrease, &QPushButton::clicked, static_cast<OverlayCounter*>(_overlay), &OverlayCounter::increase);
        connect(ui->btnDecrease, &QPushButton::clicked, static_cast<OverlayCounter*>(_overlay), &OverlayCounter::decrease);
    }

    ui->btnPlay->setEnabled(overlayType == DMHelper::OverlayType_Timer);
    if(overlayType == DMHelper::OverlayType_Timer)
    {
        connect(ui->btnPlay, &QPushButton::toggled, static_cast<OverlayTimer*>(_overlay), &OverlayTimer::toggle);
    }
}

OverlayFrame::~OverlayFrame()
{
    delete ui;
}

Overlay* OverlayFrame::getOverlay() const
{
    return _overlay;
}

void OverlayFrame::setSelected(bool selected)
{
    setStyleSheet(getStyleString(selected));
}

void OverlayFrame::handleOpacityChanged(int value)
{
    if(ui->sliderOpacity->value() != value)
        ui->sliderOpacity->setValue(value);

    if(ui->spinOpacity->value() != value)
        ui->spinOpacity->setValue(value);

    if(_overlay)
        _overlay->setOpacity(value);
}

void OverlayFrame::handleScaleSliderChanged(int value)
{
    qreal scaledValue = value / 10.0;

    if(!qFuzzyCompare(ui->spinScale->value(), scaledValue))
        ui->spinScale->setValue(scaledValue);

    if(_overlay)
        _overlay->setScale(scaledValue);
}

void OverlayFrame::handleScaleSpinChanged(qreal value)
{
    int scaledValue = static_cast<int>(value * 10.0);

    if(ui->sliderScale->value() != scaledValue)
        ui->sliderScale->setValue(scaledValue);

    if(_overlay)
        _overlay->setScale(value);
}

QString OverlayFrame::getStyleString(bool selected)
{
    if(selected)
        return QString("LayerFrame{ background-color: rgb(64, 64, 64); }");
    else
        return QString("LayerFrame{ background-color: none; }");
}
