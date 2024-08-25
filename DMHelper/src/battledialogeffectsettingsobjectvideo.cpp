#include "battledialogeffectsettingsobjectvideo.h"
#include "ui_battledialogeffectsettingsobjectvideo.h"
#include "battledialogmodeleffectobjectvideo.h"
#include "layertokens.h"
#include <QIntValidator>
#include <QGraphicsPixmapItem>
#include <QTimer>

const int BATTLEEFFECTSETTINGS_TIMER_MSEC = 1000;

BattleDialogEffectSettingsObjectVideo::BattleDialogEffectSettingsObjectVideo(const BattleDialogModelEffectObjectVideo& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsObjectVideo),
    _effect(effect),
    _color(effect.getColor()),
    _previewImage(),
    _editor(nullptr),
    _resizing(false)
{
    ui->setupUi(this);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->chkPlayAudio->setChecked(effect.isPlayAudio());
    ui->edtHeight->setValidator(new QIntValidator(1, 999, this));
    ui->edtHeight->setText(QString::number(effect.getSize()));
    ui->edtHeight->selectAll();
    ui->edtWidth->setValidator(new QIntValidator(1, 999, this));
    ui->edtWidth->setText(QString::number(effect.getWidth()));
    ui->edtWidth->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->btnEffectColor->setRotationVisible(false);
    _color.setAlpha(255);
    ui->btnEffectColor->setColor(_color);
    ui->sliderOpacity->setSliderPosition(effect.getColor().alpha());

    _editor = new TokenEditor();

    ui->btnColorizeColor->setChecked(effect.isColorize());
    _editor->setColorize(effect.isColorize());

    ui->btnColorizeColor->setRotationVisible(false);
    ui->btnColorizeColor->setColor(effect.getColorizeColor());
    _editor->setColorizeColor(effect.getColorizeColor());

    switch(effect.getEffectTransparencyType())
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

    ui->btnTransparentColor->setRotationVisible(false);
    ui->btnTransparentColor->setColor(effect.getTransparentColor());
    _editor->setTransparentColor(effect.getTransparentColor());

    ui->buttonGroup->setId(ui->btnNoTransparency, DMHelper::TransparentType_None);
    ui->buttonGroup->setId(ui->btnRed, DMHelper::TransparentType_RedChannel);
    ui->buttonGroup->setId(ui->btnGreen, DMHelper::TransparentType_GreenChannel);
    ui->buttonGroup->setId(ui->btnBlue, DMHelper::TransparentType_BlueChannel);
    ui->buttonGroup->setId(ui->btnTransparent, DMHelper::TransparentType_TransparentColor);

    ui->lblPreview->installEventFilter(this);

    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->btnTransparentColor, &QAbstractButton::setEnabled);
    connect(ui->btnTransparent, &QAbstractButton::toggled, ui->slideTolerance, &QSlider::setEnabled);
    connect(ui->buttonGroup, &QButtonGroup::idToggled, this, &BattleDialogEffectSettingsObjectVideo::handleButtonChanged);
    connect(ui->slideTolerance, &QSlider::valueChanged, this, &BattleDialogEffectSettingsObjectVideo::handleValueChanged);
    connect(ui->btnTransparentColor, &ColorPushButton::colorChanged, this, &BattleDialogEffectSettingsObjectVideo::setTransparentColor);
    connect(ui->chkColorize, &QAbstractButton::toggled, this, &BattleDialogEffectSettingsObjectVideo::setColorize);
    connect(ui->btnColorizeColor, &ColorPushButton::colorChanged, this, &BattleDialogEffectSettingsObjectVideo::setColorizeColor);

    readEffectImage();
    connect(&_effect, &BattleDialogModelEffectObjectVideo::effectReady, this, &BattleDialogEffectSettingsObjectVideo::readEffectImage);
}

BattleDialogEffectSettingsObjectVideo::~BattleDialogEffectSettingsObjectVideo()
{
    delete ui;
}

void BattleDialogEffectSettingsObjectVideo::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectObjectVideo* other = dynamic_cast<BattleDialogModelEffectObjectVideo*>(&effect);
    if(!other)
        return;

    if((!ui->edtName->text().isEmpty()) && (effect.getTip() != ui->edtName->text()))
        ui->edtName->setText(QString());

    if((!ui->chkActive->isTristate()) && (effect.getEffectActive() != ui->chkActive->isChecked()))
    {
        ui->chkActive->setTristate();
        ui->chkActive->setCheckState(Qt::PartiallyChecked);
    }

    if((!ui->chkVisible->isTristate()) && (effect.getEffectVisible() != ui->chkVisible->isChecked()))
    {
        ui->chkVisible->setTristate();
        ui->chkVisible->setCheckState(Qt::PartiallyChecked);
    }

    if((!ui->chkPlayAudio->isTristate()) && (other->isPlayAudio() != ui->chkVisible->isChecked()))
    {
        ui->chkPlayAudio->setTristate();
        ui->chkPlayAudio->setCheckState(Qt::PartiallyChecked);
    }

    if((!ui->edtHeight->text().isEmpty()) && (QString::number(effect.getSize()) != ui->edtHeight->text()))
        ui->edtHeight->setText(QString());

    if((!ui->edtWidth->text().isEmpty()) && (QString::number(effect.getWidth()) != ui->edtWidth->text()))
        ui->edtWidth->setText(QString());

    if((!ui->edtRotation->text().isEmpty()) && (QString::number(effect.getRotation()) != ui->edtRotation->text()))
        ui->edtRotation->setText(QString());

    if((!ui->chkColorize->isTristate()) && (other->getEffectVisible() != ui->chkVisible->isChecked()))
    {
        ui->chkColorize->setTristate();
        ui->chkColorize->setCheckState(Qt::PartiallyChecked);
    }
}

void BattleDialogEffectSettingsObjectVideo::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectObjectVideo* videoEffect = dynamic_cast<BattleDialogModelEffectObjectVideo*>(&effect);
    if(!videoEffect)
        return;

    if(ui->chkActive->checkState() != Qt::PartiallyChecked)
        videoEffect->setEffectActive(isEffectActive());

    if(ui->chkVisible->checkState() != Qt::PartiallyChecked)
        videoEffect->setEffectVisible(isEffectVisible());

    if(ui->chkPlayAudio->checkState() != Qt::PartiallyChecked)
        videoEffect->setPlayAudio(isPlayAudio());

    if(!ui->edtRotation->text().isEmpty())
        videoEffect->setRotation(getRotation());

    if(!ui->edtHeight->text().isEmpty())
        videoEffect->setSize(getHeightValue());

    if(!ui->edtWidth->text().isEmpty())
        videoEffect->setWidth(getWidthValue());

    if(!ui->edtName->text().isEmpty())
        videoEffect->setTip(getTip());

    QColor effectColor = ui->btnEffectColor->getColor();
    effectColor.setAlpha(ui->sliderOpacity->sliderPosition());
    videoEffect->setColor(effectColor);

    videoEffect->setColorize(ui->chkColorize->isChecked());
    videoEffect->setColorizeColor(ui->btnColorizeColor->getColor());
    videoEffect->setEffectTransparencyType(getEffectTransparencyType());
    videoEffect->setTransparentColor(ui->btnTransparentColor->getColor());
    videoEffect->setTransparentTolerance(ui->slideTolerance->value());

    if(!_editor)
        return;

    LayerTokens* tokenLayer = videoEffect->getLayer();
    if(tokenLayer)
    {
        QGraphicsPixmapItem* pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(tokenLayer->getEffectItem(videoEffect));
        if(pixmapItem)
        {
            _editor->setSquareFinalImage(false);
            _editor->setSourceImage(_previewImage);
            QPixmap itemPixmap = QPixmap::fromImage(_editor->getFinalImage());
            pixmapItem->setPixmap(itemPixmap);
            pixmapItem->setOffset(-itemPixmap.width() / 2, -itemPixmap.height() / 2);
            setEditorSource();
        }
    }
}

bool BattleDialogEffectSettingsObjectVideo::isEffectActive() const
{
    return ui->chkActive->isChecked();
}

bool BattleDialogEffectSettingsObjectVideo::isEffectVisible() const
{
    return ui->chkVisible->isChecked();
}

bool BattleDialogEffectSettingsObjectVideo::isPlayAudio() const
{
    return ui->chkPlayAudio->isChecked();
}

QString BattleDialogEffectSettingsObjectVideo::getTip() const
{
    return ui->edtName->text();
}

int BattleDialogEffectSettingsObjectVideo::getHeightValue() const
{
    return ui->edtHeight->text().toInt();
}

int BattleDialogEffectSettingsObjectVideo::getWidthValue() const
{
    return ui->edtWidth->text().toInt();
}

qreal BattleDialogEffectSettingsObjectVideo::getRotation() const
{
    return ui->edtRotation->text().toDouble();
}

DMHelper::TransparentType BattleDialogEffectSettingsObjectVideo::getEffectTransparencyType() const
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

QColor BattleDialogEffectSettingsObjectVideo::getTransparentColor() const
{
    return ui->btnTransparentColor->getColor();
}

qreal BattleDialogEffectSettingsObjectVideo::getTransparentTolerance() const
{
    return static_cast<qreal>(ui->slideTolerance->value()) / 100.0;
}

bool BattleDialogEffectSettingsObjectVideo::isColorize() const
{
    return ui->chkColorize->isChecked();
}

QColor BattleDialogEffectSettingsObjectVideo::getColorizeColor() const
{
    return ui->btnColorizeColor->getColor();
}

bool BattleDialogEffectSettingsObjectVideo::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->lblPreview) && (event) && (event->type() == QEvent::Resize) && (!_resizing))
    {
        _resizing = true;
        QTimer::singleShot(BATTLEEFFECTSETTINGS_TIMER_MSEC, this, &BattleDialogEffectSettingsObjectVideo::setEditorSource);
    }

    return QDialog::eventFilter(watched, event);
}

void BattleDialogEffectSettingsObjectVideo::readEffectImage()
{
    _previewImage = _effect.getPixmap().toImage();
    setEditorSource();
}

void BattleDialogEffectSettingsObjectVideo::setTransparentColor(const QColor& transparentColor)
{
    ui->btnTransparentColor->setColor(transparentColor);
    _editor->setTransparentColor(transparentColor);
    updatePreview();
}

void BattleDialogEffectSettingsObjectVideo::setColorize(bool colorize)
{
    ui->chkColorize->setChecked(colorize);
    _editor->setColorize(colorize);
    updatePreview();
}

void BattleDialogEffectSettingsObjectVideo::setColorizeColor(const QColor& colorizeColor)
{
    ui->btnColorizeColor->setColor(colorizeColor);
    _editor->setColorizeColor(colorizeColor);
    updatePreview();
}

void BattleDialogEffectSettingsObjectVideo::handleButtonChanged(int id, bool checked)
{
    if(checked)
    {
        _editor->setTransparentValue(static_cast<DMHelper::TransparentType>(id));
        updatePreview();
    }
}

void BattleDialogEffectSettingsObjectVideo::handleValueChanged(int value)
{
    _editor->setTransparentLevel(value);
    updatePreview();
}

void BattleDialogEffectSettingsObjectVideo::setEditorSource()
{
    if(_previewImage.isNull())
        return;

    QSize previewSize = ui->lblPreview->size() - QSize(10, 10);
    if(!previewSize.isValid())
        return;

    _editor->setSquareFinalImage(true);
    _editor->setSourceImage(_previewImage.scaled(previewSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    updatePreview();

    _resizing = false;
}

void BattleDialogEffectSettingsObjectVideo::updatePreview()
{
    ui->lblPreview->setPixmap(QPixmap::fromImage(_editor->getFinalImage()));
}
