#include "battledialogeffectsettingssparks.h"
#include "ui_battledialogeffectsettingssparks.h"
#include "battledialogmodeleffectsparks.h"
#include <QComboBox>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QSignalBlocker>

static constexpr int PRESET_INDEX_CUSTOM = -1;
static constexpr int PRESET_INDEX_TORCH = 0;
static constexpr int PRESET_INDEX_ARCANE = 1;
static constexpr int PRESET_INDEX_ARCANE_WILD = 2;
static constexpr int PRESET_INDEX_FORGE_STRIKE = 3;

BattleDialogEffectSettingsSparks::BattleDialogEffectSettingsSparks(const BattleDialogModelEffectSparks& effect, QWidget *parent) :
    BattleDialogEffectSettingsBase(parent),
    ui(new Ui::BattleDialogEffectSettingsSparks)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    ui->edtName->setText(effect.getTip());
    ui->chkActive->setChecked(effect.getEffectActive());
    ui->chkVisible->setChecked(effect.getEffectVisible());
    ui->edtSize->setValidator(new QIntValidator(1, 999, this));
    ui->edtSize->setText(QString::number(effect.getSize()));
    ui->edtSize->selectAll();
    ui->edtRotation->setValidator(new QDoubleValidator(0, 360, 5, this));
    ui->edtRotation->setText(QString::number(effect.getRotation()));
    ui->sliderOpacity->setSliderPosition(effect.getColor().alpha());

    ui->btnColor->setRotationVisible(false);
    QColor sparkColor = effect.getColor();
    sparkColor.setAlpha(255);
    ui->btnColor->setColor(sparkColor);

    ui->spinParticleCount->setValue(effect.getParticleCount());
    ui->spinGlowRadius->setValue(effect.getGlowRadius());
    ui->spinGlowOpacity->setValue(effect.getGlowOpacity());
    ui->chkArcFalloff->setChecked(effect.getArcFalloff());
    ui->chkFadeDistance->setChecked(effect.getFadeDistance());
    ui->spinSparkSpeed->setValue(effect.getSparkSpeed());
    ui->spinWindDirection->setValue(effect.getWindDirection());
    ui->spinWindStrength->setValue(effect.getWindStrength());

    initializePresets();
    connect(ui->cmbPresets, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        applyPreset(ui->cmbPresets->itemData(index).toInt());
    });
}

BattleDialogEffectSettingsSparks::~BattleDialogEffectSettingsSparks()
{
    delete ui;
}

void BattleDialogEffectSettingsSparks::mergeValuesToSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSparks* other = dynamic_cast<BattleDialogModelEffectSparks*>(&effect);
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

    if((!ui->edtSize->text().isEmpty()) && (QString::number(effect.getSize()) != ui->edtSize->text()))
        ui->edtSize->setText(QString());

    if((!ui->edtRotation->text().isEmpty()) && (QString::number(effect.getRotation()) != ui->edtRotation->text()))
        ui->edtRotation->setText(QString());
}

void BattleDialogEffectSettingsSparks::copyValuesFromSettings(BattleDialogModelEffect& effect)
{
    BattleDialogModelEffectSparks* sparksEffect = dynamic_cast<BattleDialogModelEffectSparks*>(&effect);
    if(!sparksEffect)
        return;

    if(ui->chkActive->checkState() != Qt::PartiallyChecked)
        effect.setEffectActive(ui->chkActive->isChecked());

    if(ui->chkVisible->checkState() != Qt::PartiallyChecked)
        effect.setEffectVisible(ui->chkVisible->isChecked());

    if(!ui->edtRotation->text().isEmpty())
        effect.setRotation(ui->edtRotation->text().toDouble());

    if(!ui->edtSize->text().isEmpty())
        effect.setSize(ui->edtSize->text().toInt());

    if(!ui->edtName->text().isEmpty())
        effect.setTip(ui->edtName->text());

    QColor effectColor = ui->btnColor->getColor();
    effectColor.setAlpha(ui->sliderOpacity->sliderPosition());
    effect.setColor(effectColor);

    sparksEffect->setParticleCount(ui->spinParticleCount->value());
    sparksEffect->setGlowRadius(ui->spinGlowRadius->value());
    sparksEffect->setGlowOpacity(ui->spinGlowOpacity->value());
    sparksEffect->setArcFalloff(ui->chkArcFalloff->isChecked());
    sparksEffect->setFadeDistance(ui->chkFadeDistance->isChecked());
    sparksEffect->setSparkSpeed(ui->spinSparkSpeed->value());
    sparksEffect->setWindDirection(ui->spinWindDirection->value());
    sparksEffect->setWindStrength(ui->spinWindStrength->value());
}

void BattleDialogEffectSettingsSparks::initializePresets()
{
    const QSignalBlocker blocker(ui->cmbPresets);

    ui->cmbPresets->clear();
    ui->cmbPresets->addItem(tr("Custom"), PRESET_INDEX_CUSTOM);
    ui->cmbPresets->addItem(tr("Torch"), PRESET_INDEX_TORCH);
    ui->cmbPresets->addItem(tr("Arcane"), PRESET_INDEX_ARCANE);
    ui->cmbPresets->addItem(tr("Arcane Wild"), PRESET_INDEX_ARCANE_WILD);
    ui->cmbPresets->addItem(tr("Forge Strike"), PRESET_INDEX_FORGE_STRIKE);
    ui->cmbPresets->setCurrentIndex(0);
}

void BattleDialogEffectSettingsSparks::applyPreset(int presetIndex)
{
    SparksPreset preset;
    bool validPreset = true;

    switch(presetIndex)
    {
    case PRESET_INDEX_TORCH:
        preset = {
            tr("Torch"),
            10,
            QColor(255, 185, 75),
            235,
            70,
            0.35,
            0.75,
            true,
            true,
            0.45,
            90.0,
            0.12
        };
        break;
    case PRESET_INDEX_ARCANE:
        preset = {
            tr("Arcane"),
            12,
            QColor(95, 220, 255),
            240,
            130,
            0.60,
            0.95,
            false,
            false,
            0.60,
            45.0,
            0.18
        };
        break;
    case PRESET_INDEX_ARCANE_WILD:
        preset = {
            tr("Arcane Wild"),
            14,
            QColor(175, 95, 255),
            255,
            180,
            0.72,
            1.00,
            false,
            true,
            1.00,
            30.0,
            0.55
        };
        break;
    case PRESET_INDEX_FORGE_STRIKE:
        preset = {
            tr("Forge Strike"),
            12,
            QColor(255, 210, 110),
            255,
            160,
            0.42,
            0.92,
            true,
            true,
            1.15,
            0.0,
            0.35
        };
        break;
    default:
        validPreset = false;
        break;
    }

    if(!validPreset)
        return;

    ui->edtSize->setText(QString::number(preset.size));
    ui->btnColor->setColor(preset.color);
    ui->sliderOpacity->setSliderPosition(preset.opacity);
    ui->spinParticleCount->setValue(preset.particleCount);
    ui->spinGlowRadius->setValue(preset.glowRadius);
    ui->spinGlowOpacity->setValue(preset.glowOpacity);
    ui->chkArcFalloff->setChecked(preset.arcFalloff);
    ui->chkFadeDistance->setChecked(preset.fadeDistance);
    ui->spinSparkSpeed->setValue(preset.sparkSpeed);
    ui->spinWindDirection->setValue(preset.windDirection);
    ui->spinWindStrength->setValue(preset.windStrength);
}
