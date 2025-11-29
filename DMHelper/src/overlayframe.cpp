#include "overlayframe.h"
#include "ui_overlayframe.h"
#include "overlay.h"
#include "overlaycounter.h"
#include "overlaytimer.h"
#include "dmconstants.h"
#include <QHBoxLayout>
#include <QDebug>

const int OVERLAY_FRAME_INSERT_POINT = 3;

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

    ui->edtName->setText(_overlay->objectName());
    ui->chkVisible->setChecked(_overlay->isVisible());
    connect(ui->edtName, &QLineEdit::editingFinished, this, &OverlayFrame::handleNameChanged);
    connect(ui->chkVisible, &QCheckBox::toggled, _overlay, &Overlay::setVisible);

    _overlay->prepareFrame(dynamic_cast<QBoxLayout*>(layout()), OVERLAY_FRAME_INSERT_POINT);

    // TEMP TEMP TEMP - remove these programmatically
    ui->spinScale->setValue(_overlay->getScale());
    ui->sliderScale->setValue(static_cast<int>(_overlay->getScale() * 10.0));
    ui->spinOpacity->setValue(_overlay->getOpacity());
    ui->sliderOpacity->setValue(_overlay->getOpacity());
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

    // TEMP TEMP TEMP - remove these programmatically
    ui->btnPlay->hide();
    ui->btnIncrease->hide();
    ui->btnDecrease->hide();
    ui->lblScale->hide();
    ui->lblOpacity->hide();
    ui->sliderScale->hide();
    ui->sliderOpacity->hide();
    ui->spinOpacity->hide();
    ui->spinScale->hide();
    ui->btnSettings->hide();

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

QHBoxLayout* OverlayFrame::getLayout() const
{
    return dynamic_cast<QHBoxLayout*>(layout());
}

void OverlayFrame::handleNameChanged()
{
    if((_overlay) && (!ui->edtName->text().isEmpty()) && (_overlay->objectName() != ui->edtName->text()))
        _overlay->setObjectName(ui->edtName->text());
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
        return QString("OverlayFrame{ background-color: rgb(64, 64, 64); }");
    else
        return QString("OverlayFrame{ background-color: none; }");
}
