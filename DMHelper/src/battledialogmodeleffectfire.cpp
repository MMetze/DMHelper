#include "battledialogmodeleffectfire.h"
#include "battledialogeffectsettingsfire.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QPainter>
#include <QLinearGradient>

static constexpr qreal DEFAULT_INTENSITY = 0.5;
static const QColor DEFAULT_DARK_COLOR(180, 30, 0, 220);
static const QColor DEFAULT_LIGHT_COLOR(255, 200, 50, 200);
static constexpr qreal DEFAULT_FLICKER_SPEED = 0.5;
static constexpr int PREVIEW_PIXMAP_SIZE = 200;
static constexpr qreal GRADIENT_DARK_STOP = 0.6;

BattleDialogModelEffectFire::BattleDialogModelEffectFire(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _intensity(DEFAULT_INTENSITY),
    _darkColor(DEFAULT_DARK_COLOR),
    _lightColor(DEFAULT_LIGHT_COLOR),
    _flickerSpeed(DEFAULT_FLICKER_SPEED)
{
}

BattleDialogModelEffectFire::BattleDialogModelEffectFire(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip),
    _intensity(DEFAULT_INTENSITY),
    _darkColor(DEFAULT_DARK_COLOR),
    _lightColor(DEFAULT_LIGHT_COLOR),
    _flickerSpeed(DEFAULT_FLICKER_SPEED)
{
}

BattleDialogModelEffectFire::~BattleDialogModelEffectFire()
{
}

void BattleDialogModelEffectFire::inputXML(const QDomElement &element, bool isImport)
{
    _intensity = element.attribute("intensity", QString::number(DEFAULT_INTENSITY)).toDouble();
    _darkColor = QColor(element.attribute("darkColorR", QString::number(DEFAULT_DARK_COLOR.red())).toInt(),
                        element.attribute("darkColorG", QString::number(DEFAULT_DARK_COLOR.green())).toInt(),
                        element.attribute("darkColorB", QString::number(DEFAULT_DARK_COLOR.blue())).toInt(),
                        element.attribute("darkColorA", QString::number(DEFAULT_DARK_COLOR.alpha())).toInt());
    _lightColor = QColor(element.attribute("lightColorR", QString::number(DEFAULT_LIGHT_COLOR.red())).toInt(),
                         element.attribute("lightColorG", QString::number(DEFAULT_LIGHT_COLOR.green())).toInt(),
                         element.attribute("lightColorB", QString::number(DEFAULT_LIGHT_COLOR.blue())).toInt(),
                         element.attribute("lightColorA", QString::number(DEFAULT_LIGHT_COLOR.alpha())).toInt());
    _flickerSpeed = element.attribute("flickerSpeed", QString::number(DEFAULT_FLICKER_SPEED)).toDouble();

    BattleDialogModelEffect::inputXML(element, isImport);
}

void BattleDialogModelEffectFire::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectFire* otherEffect = dynamic_cast<const BattleDialogModelEffectFire*>(other);
    if(!otherEffect)
        return;

    _intensity = otherEffect->_intensity;
    _darkColor = otherEffect->_darkColor;
    _lightColor = otherEffect->_lightColor;
    _flickerSpeed = otherEffect->_flickerSpeed;

    BattleDialogModelEffect::copyValues(other);
}

QString BattleDialogModelEffectFire::getName() const
{
    return _tip.isEmpty() ? QString("Fire Effect") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectFire::clone() const
{
    BattleDialogModelEffectFire* newEffect = new BattleDialogModelEffectFire(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectFire::getEffectType() const
{
    return BattleDialogModelEffect_Fire;
}

BattleDialogEffectSettingsBase* BattleDialogModelEffectFire::getEffectEditor() const
{
    return new BattleDialogEffectSettingsFire(*this);
}

QGraphicsItem* BattleDialogModelEffectFire::createEffectShape(qreal gridScale)
{
    int previewSize = PREVIEW_PIXMAP_SIZE;
    QPixmap pixmap(previewSize, previewSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(previewSize / 2.0, previewSize / 2.0, previewSize / 2.0);
    gradient.setColorAt(0.0, _lightColor);
    gradient.setColorAt(GRADIENT_DARK_STOP, _darkColor);
    gradient.setColorAt(1.0, QColor(_darkColor.red(), _darkColor.green(), _darkColor.blue(), 0));
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

qreal BattleDialogModelEffectFire::getIntensity() const
{
    return _intensity;
}

void BattleDialogModelEffectFire::setIntensity(qreal intensity)
{
    if(!qFuzzyCompare(_intensity, intensity))
    {
        _intensity = intensity;
        registerChange();
    }
}

QColor BattleDialogModelEffectFire::getDarkColor() const
{
    return _darkColor;
}

void BattleDialogModelEffectFire::setDarkColor(const QColor& color)
{
    if(_darkColor != color)
    {
        _darkColor = color;
        registerChange();
    }
}

QColor BattleDialogModelEffectFire::getLightColor() const
{
    return _lightColor;
}

void BattleDialogModelEffectFire::setLightColor(const QColor& color)
{
    if(_lightColor != color)
    {
        _lightColor = color;
        registerChange();
    }
}

qreal BattleDialogModelEffectFire::getFlickerSpeed() const
{
    return _flickerSpeed;
}

void BattleDialogModelEffectFire::setFlickerSpeed(qreal flickerSpeed)
{
    if(!qFuzzyCompare(_flickerSpeed, flickerSpeed))
    {
        _flickerSpeed = flickerSpeed;
        registerChange();
    }
}

void BattleDialogModelEffectFire::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("intensity", _intensity);
    element.setAttribute("darkColorR", _darkColor.red());
    element.setAttribute("darkColorG", _darkColor.green());
    element.setAttribute("darkColorB", _darkColor.blue());
    element.setAttribute("darkColorA", _darkColor.alpha());
    element.setAttribute("lightColorR", _lightColor.red());
    element.setAttribute("lightColorG", _lightColor.green());
    element.setAttribute("lightColorB", _lightColor.blue());
    element.setAttribute("lightColorA", _lightColor.alpha());
    element.setAttribute("flickerSpeed", _flickerSpeed);

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}
