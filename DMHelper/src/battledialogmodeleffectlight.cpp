#include "battledialogmodeleffectlight.h"
#include "battledialogeffectsettingslight.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QPainter>
#include <QRadialGradient>

static constexpr qreal DEFAULT_FLICKER_FREQUENCY = 2.0;
static constexpr qreal DEFAULT_DIM_AMPLITUDE = 0.2;
static const QColor DEFAULT_LIGHT_COLOR(255, 220, 100, 128);
static constexpr int PREVIEW_PIXMAP_SIZE = 200;
static constexpr qreal QUADRATIC_STOP_INNER = 0.3;
static constexpr qreal QUADRATIC_STOP_MID = 0.5;
static constexpr qreal QUADRATIC_STOP_OUTER = 0.7;
static constexpr qreal QUADRATIC_ALPHA_INNER = 0.91;
static constexpr qreal QUADRATIC_ALPHA_MID = 0.75;
static constexpr qreal QUADRATIC_ALPHA_OUTER = 0.51;

BattleDialogModelEffectLight::BattleDialogModelEffectLight(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _gradientType(GradientType_Linear),
    _flickerEnabled(false),
    _flickerFrequency(DEFAULT_FLICKER_FREQUENCY),
    _dimAmplitude(DEFAULT_DIM_AMPLITUDE)
{
    _color = DEFAULT_LIGHT_COLOR;
}

BattleDialogModelEffectLight::BattleDialogModelEffectLight(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip),
    _gradientType(GradientType_Linear),
    _flickerEnabled(false),
    _flickerFrequency(DEFAULT_FLICKER_FREQUENCY),
    _dimAmplitude(DEFAULT_DIM_AMPLITUDE)
{
}

BattleDialogModelEffectLight::~BattleDialogModelEffectLight()
{
}

void BattleDialogModelEffectLight::inputXML(const QDomElement &element, bool isImport)
{
    _gradientType = element.attribute("gradientType", QString::number(GradientType_Linear)).toInt();
    _flickerEnabled = static_cast<bool>(element.attribute("flickerEnabled", QString::number(0)).toInt());
    _flickerFrequency = element.attribute("flickerFrequency", QString::number(DEFAULT_FLICKER_FREQUENCY)).toDouble();
    _dimAmplitude = element.attribute("dimAmplitude", QString::number(DEFAULT_DIM_AMPLITUDE)).toDouble();

    BattleDialogModelEffect::inputXML(element, isImport);
}

void BattleDialogModelEffectLight::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectLight* otherEffect = dynamic_cast<const BattleDialogModelEffectLight*>(other);
    if(!otherEffect)
        return;

    _gradientType = otherEffect->_gradientType;
    _flickerEnabled = otherEffect->_flickerEnabled;
    _flickerFrequency = otherEffect->_flickerFrequency;
    _dimAmplitude = otherEffect->_dimAmplitude;

    BattleDialogModelEffect::copyValues(other);
}

QString BattleDialogModelEffectLight::getName() const
{
    return _tip.isEmpty() ? QString("Light Effect") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectLight::clone() const
{
    BattleDialogModelEffectLight* newEffect = new BattleDialogModelEffectLight(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectLight::getEffectType() const
{
    return BattleDialogModelEffect_Light;
}

BattleDialogEffectSettingsBase* BattleDialogModelEffectLight::getEffectEditor() const
{
    return new BattleDialogEffectSettingsLight(*this);
}

QGraphicsItem* BattleDialogModelEffectLight::createEffectShape(qreal gridScale)
{
    int previewSize = PREVIEW_PIXMAP_SIZE;
    QPixmap pixmap(previewSize, previewSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(previewSize / 2.0, previewSize / 2.0, previewSize / 2.0);
    gradient.setColorAt(0.0, _color);

    if(_gradientType == GradientType_Quadratic)
    {
        gradient.setColorAt(QUADRATIC_STOP_INNER, QColor(_color.red(), _color.green(), _color.blue(), static_cast<int>(_color.alpha() * QUADRATIC_ALPHA_INNER)));
        gradient.setColorAt(QUADRATIC_STOP_MID, QColor(_color.red(), _color.green(), _color.blue(), static_cast<int>(_color.alpha() * QUADRATIC_ALPHA_MID)));
        gradient.setColorAt(QUADRATIC_STOP_OUTER, QColor(_color.red(), _color.green(), _color.blue(), static_cast<int>(_color.alpha() * QUADRATIC_ALPHA_OUTER)));
    }
    else
    {
        gradient.setColorAt(0.5, QColor(_color.red(), _color.green(), _color.blue(), _color.alpha() / 2));
    }
    gradient.setColorAt(1.0, QColor(_color.red(), _color.green(), _color.blue(), 0));

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, previewSize, previewSize);
    painter.end();

    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(this);
    pixmapItem->setPixmap(pixmap);
    pixmapItem->setOffset(-previewSize / 2, -previewSize / 2);

    setEffectItemData(pixmapItem);
    prepareItem(*pixmapItem);
    applyEffectValues(*pixmapItem, gridScale);

    return pixmapItem;
}

int BattleDialogModelEffectLight::getGradientType() const
{
    return _gradientType;
}

void BattleDialogModelEffectLight::setGradientType(int gradientType)
{
    if(_gradientType != gradientType)
    {
        _gradientType = gradientType;
        registerChange();
    }
}

bool BattleDialogModelEffectLight::getFlickerEnabled() const
{
    return _flickerEnabled;
}

void BattleDialogModelEffectLight::setFlickerEnabled(bool enabled)
{
    if(_flickerEnabled != enabled)
    {
        _flickerEnabled = enabled;
        registerChange();
    }
}

qreal BattleDialogModelEffectLight::getFlickerFrequency() const
{
    return _flickerFrequency;
}

void BattleDialogModelEffectLight::setFlickerFrequency(qreal frequency)
{
    if(!qFuzzyCompare(_flickerFrequency, frequency))
    {
        _flickerFrequency = frequency;
        registerChange();
    }
}

qreal BattleDialogModelEffectLight::getDimAmplitude() const
{
    return _dimAmplitude;
}

void BattleDialogModelEffectLight::setDimAmplitude(qreal amplitude)
{
    if(!qFuzzyCompare(_dimAmplitude, amplitude))
    {
        _dimAmplitude = amplitude;
        registerChange();
    }
}

void BattleDialogModelEffectLight::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("gradientType", _gradientType);
    element.setAttribute("flickerEnabled", static_cast<int>(_flickerEnabled));
    element.setAttribute("flickerFrequency", _flickerFrequency);
    element.setAttribute("dimAmplitude", _dimAmplitude);

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}
