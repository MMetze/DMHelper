#include "layervideoeffectsettings.h"
#include "ui_layervideoeffectsettings.h"
#include "colorpushbutton.h"

LayerVideoEffectSettings::LayerVideoEffectSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerVideoEffectSettings),
    _previewImage(),
    _editor(nullptr)
{
    ui->setupUi(this);

    ui->btnTransparentColor->setRotationVisible(false);
    ui->btnColorizeColor->setRotationVisible(false);

    ui->btnNoTransparency->setChecked(true);
    ui->btnColorizeColor->setChecked(false);

    ui->grpTransparency->setId(ui->btnNoTransparency, DMHelper::TransparentType_None);
    ui->grpTransparency->setId(ui->btnRed, DMHelper::TransparentType_RedChannel);
    ui->grpTransparency->setId(ui->btnGreen, DMHelper::TransparentType_GreenChannel);
    ui->grpTransparency->setId(ui->btnBlue, DMHelper::TransparentType_BlueChannel);
    ui->grpTransparency->setId(ui->btnTransparent, DMHelper::TransparentType_TransparentColor);

    ui->lblPreview->installEventFilter(this);

    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->btnTransparentColor, &QAbstractButton::setEnabled);
    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->slideTolerance, &QSlider::setEnabled);
    connect(ui->grpTransparency, &QButtonGroup::idToggled, this, &LayerVideoEffectSettings::handleButtonChanged);
    connect(ui->slideTolerance, &QSlider::valueChanged, this, &LayerVideoEffectSettings::handleValueChanged);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, this, &LayerVideoEffectSettings::setTransparentColor);
    connect(ui->chkColorize, &QAbstractButton::toggled, this, &LayerVideoEffectSettings::setColorize);
    connect(ui->btnColorizeColor, &ColorPushButton::colorChanged, this, &LayerVideoEffectSettings::setColorizeColor);

    _editor = new TokenEditor();
}

LayerVideoEffectSettings::~LayerVideoEffectSettings()
{
    _editor->deleteLater();
    delete ui;
}

DMHelper::TransparentType LayerVideoEffectSettings::getEffectType() const
{
    if(ui->btnRed->isChecked())
        return DMHelper::TransparentType_RedChannel;
    else if(ui->btnGreen->isChecked())
        return DMHelper::TransparentType_GreenChannel;
    else if(ui->btnBlue->isChecked())
        return DMHelper::TransparentType_BlueChannel;
    else if(ui->btnTransparent->isChecked())
        return DMHelper::TransparentType_TransparentColor;
    else
        return DMHelper::TransparentType_None;
}

QColor LayerVideoEffectSettings::getTransparentColor() const
{
    return ui->btnTransparentColor->getColor();
}

qreal LayerVideoEffectSettings::getTransparentTolerance() const
{
    return static_cast<qreal>(ui->slideTolerance->value()) / 100.0;
}

bool LayerVideoEffectSettings::isColorize() const
{
    return ui->chkColorize->isChecked();
}

QColor LayerVideoEffectSettings::getColorizeColor() const
{
    return ui->btnColorizeColor->getColor();
}

bool LayerVideoEffectSettings::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->lblPreview) && (event) && (event->type() == QEvent::Resize))
    {
        setEditorSource();
    }

    return QDialog::eventFilter(watched, event);
}

void LayerVideoEffectSettings::setEffectType(DMHelper::TransparentType effectType)
{
    switch(effectType)
    {
        case DMHelper::TransparentType_RedChannel:
            ui->btnRed->setChecked(true);
            _editor->setTransparentValue(DMHelper::TransparentType_RedChannel);
            break;
        case DMHelper::TransparentType_GreenChannel:
            ui->btnGreen->setChecked(true);
            _editor->setTransparentValue(DMHelper::TransparentType_GreenChannel);
            break;
        case DMHelper::TransparentType_BlueChannel:
            ui->btnBlue->setChecked(true);
            _editor->setTransparentValue(DMHelper::TransparentType_BlueChannel);
            break;
        case DMHelper::TransparentType_TransparentColor:
            ui->btnTransparent->setChecked(true);
            _editor->setTransparentValue(DMHelper::TransparentType_TransparentColor);
            break;
        default:
            ui->btnNoTransparency->setChecked(true);
            _editor->setTransparentValue(DMHelper::TransparentType_None);
            break;
    }

    updatePreview();
}

void LayerVideoEffectSettings::setTransparentColor(const QColor& transparentColor)
{
    ui->btnTransparentColor->setColor(transparentColor);
    _editor->setTransparentColor(transparentColor);
    updatePreview();
}

void LayerVideoEffectSettings::setTransparentTolerance(qreal transparentTolerance)
{
    int intTolerance = static_cast<int>(transparentTolerance * 100.0);
    ui->slideTolerance->setValue(intTolerance);
    _editor->setTransparentLevel(intTolerance);
    updatePreview();
}

void LayerVideoEffectSettings::setColorize(bool colorize)
{
    ui->chkColorize->setChecked(colorize);
    _editor->setColorize(colorize);
    updatePreview();
}

void LayerVideoEffectSettings::setColorizeColor(const QColor& colorizeColor)
{
    ui->btnColorizeColor->setColor(colorizeColor);
    _editor->setColorizeColor(colorizeColor);
    updatePreview();
}

void LayerVideoEffectSettings::setPreviewImage(const QImage& image)
{
    _previewImage = image;
    setEditorSource();
}

void LayerVideoEffectSettings::handleButtonChanged(int id, bool checked)
{
    if(checked)
    {
        _editor->setTransparentValue(static_cast<DMHelper::TransparentType>(id));
        updatePreview();
    }
}

void LayerVideoEffectSettings::handleValueChanged(int value)
{
    _editor->setTransparentLevel(value);
    updatePreview();
}

void LayerVideoEffectSettings::setEditorSource()
{
    if(_previewImage.isNull())
        return;

    QSize previewSize = ui->lblPreview->size() - QSize(10, 10);
    if(!previewSize.isValid())
        return;

    _editor->setSourceImage(_previewImage.scaled(previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    updatePreview();
}

void LayerVideoEffectSettings::updatePreview()
{
    ui->lblPreview->setPixmap(QPixmap::fromImage(_editor->getFinalImage()));
}
