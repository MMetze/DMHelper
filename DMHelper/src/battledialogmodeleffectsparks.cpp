#include "battledialogmodeleffectsparks.h"
#include "battledialogeffectsettingssparks.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QPainter>
#include <QRandomGenerator>

static constexpr int DEFAULT_PARTICLE_COUNT = 50;
static constexpr qreal DEFAULT_GLOW_RADIUS = 0.3;
static constexpr qreal DEFAULT_GLOW_OPACITY = 0.6;
static constexpr qreal DEFAULT_SPARK_SPEED = 0.5;
static constexpr qreal DEFAULT_WIND_DIRECTION = 0.0;
static constexpr qreal DEFAULT_WIND_STRENGTH = 0.0;
static const QColor DEFAULT_SPARK_COLOR(255, 200, 50, 255);
static constexpr int PREVIEW_PIXMAP_SIZE = 200;
static constexpr int PREVIEW_MAX_DOTS = 100;
static constexpr qreal PREVIEW_DOT_MIN_SIZE = 2.0;
static constexpr qreal PREVIEW_DOT_SIZE_RANGE = 3.0;

BattleDialogModelEffectSparks::BattleDialogModelEffectSparks(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _particleCount(DEFAULT_PARTICLE_COUNT),
    _glowRadius(DEFAULT_GLOW_RADIUS),
    _glowOpacity(DEFAULT_GLOW_OPACITY),
    _arcFalloff(true),
    _fadeDistance(true),
    _sparkSpeed(DEFAULT_SPARK_SPEED),
    _windDirection(DEFAULT_WIND_DIRECTION),
    _windStrength(DEFAULT_WIND_STRENGTH)
{
    _color = DEFAULT_SPARK_COLOR;
}

BattleDialogModelEffectSparks::BattleDialogModelEffectSparks(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip),
    _particleCount(DEFAULT_PARTICLE_COUNT),
    _glowRadius(DEFAULT_GLOW_RADIUS),
    _glowOpacity(DEFAULT_GLOW_OPACITY),
    _arcFalloff(true),
    _fadeDistance(true),
    _sparkSpeed(DEFAULT_SPARK_SPEED),
    _windDirection(DEFAULT_WIND_DIRECTION),
    _windStrength(DEFAULT_WIND_STRENGTH)
{
}

BattleDialogModelEffectSparks::~BattleDialogModelEffectSparks()
{
}

void BattleDialogModelEffectSparks::inputXML(const QDomElement &element, bool isImport)
{
    _particleCount = element.attribute("particleCount", QString::number(DEFAULT_PARTICLE_COUNT)).toInt();
    _glowRadius = element.attribute("glowRadius", QString::number(DEFAULT_GLOW_RADIUS)).toDouble();
    _glowOpacity = element.attribute("glowOpacity", QString::number(DEFAULT_GLOW_OPACITY)).toDouble();
    _arcFalloff = static_cast<bool>(element.attribute("arcFalloff", QString::number(1)).toInt());
    _fadeDistance = static_cast<bool>(element.attribute("fadeDistance", QString::number(1)).toInt());
    _sparkSpeed = element.attribute("sparkSpeed", QString::number(DEFAULT_SPARK_SPEED)).toDouble();
    _windDirection = element.attribute("windDirection", QString::number(DEFAULT_WIND_DIRECTION)).toDouble();
    _windStrength = element.attribute("windStrength", QString::number(DEFAULT_WIND_STRENGTH)).toDouble();

    BattleDialogModelEffect::inputXML(element, isImport);
}

void BattleDialogModelEffectSparks::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectSparks* otherEffect = dynamic_cast<const BattleDialogModelEffectSparks*>(other);
    if(!otherEffect)
        return;

    _particleCount = otherEffect->_particleCount;
    _glowRadius = otherEffect->_glowRadius;
    _glowOpacity = otherEffect->_glowOpacity;
    _arcFalloff = otherEffect->_arcFalloff;
    _fadeDistance = otherEffect->_fadeDistance;
    _sparkSpeed = otherEffect->_sparkSpeed;
    _windDirection = otherEffect->_windDirection;
    _windStrength = otherEffect->_windStrength;

    BattleDialogModelEffect::copyValues(other);
}

QString BattleDialogModelEffectSparks::getName() const
{
    return _tip.isEmpty() ? QString("Sparks Effect") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectSparks::clone() const
{
    BattleDialogModelEffectSparks* newEffect = new BattleDialogModelEffectSparks(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectSparks::getEffectType() const
{
    return BattleDialogModelEffect_Sparks;
}

BattleDialogEffectSettingsBase* BattleDialogModelEffectSparks::getEffectEditor() const
{
    return new BattleDialogEffectSettingsSparks(*this);
}

QGraphicsItem* BattleDialogModelEffectSparks::createEffectShape(qreal gridScale)
{
    int previewSize = PREVIEW_PIXMAP_SIZE;
    QPixmap pixmap(previewSize, previewSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRandomGenerator* rng = QRandomGenerator::global();
    int dotCount = qMin(_particleCount, PREVIEW_MAX_DOTS);
    qreal cx = previewSize / 2.0;
    qreal cy = previewSize / 2.0;
    qreal maxR = previewSize / 2.0;

    for(int i = 0; i < dotCount; ++i)
    {
        qreal angle = rng->bounded(360.0) * M_PI / 180.0;
        qreal dist = rng->bounded(1.0) * maxR;
        qreal x = cx + dist * cos(angle);
        qreal y = cy + dist * sin(angle);
        qreal dotSize = PREVIEW_DOT_MIN_SIZE + rng->bounded(PREVIEW_DOT_SIZE_RANGE);

        QColor dotColor = _color;
        if(_fadeDistance)
            dotColor.setAlphaF(dotColor.alphaF() * (1.0 - dist / maxR));

        painter.setPen(Qt::NoPen);
        painter.setBrush(dotColor);
        painter.drawEllipse(QPointF(x, y), dotSize, dotSize);
    }
    painter.end();

    QGraphicsPixmapItem* pixmapItem = new UnselectedPixmap(this);
    pixmapItem->setPixmap(pixmap);
    pixmapItem->setOffset(-previewSize / 2, -previewSize / 2);

    setEffectItemData(pixmapItem);
    prepareItem(*pixmapItem);
    applyEffectValues(*pixmapItem, gridScale);

    return pixmapItem;
}

int BattleDialogModelEffectSparks::getParticleCount() const
{
    return _particleCount;
}

void BattleDialogModelEffectSparks::setParticleCount(int count)
{
    if(_particleCount != count)
    {
        _particleCount = count;
        registerChange();
    }
}

qreal BattleDialogModelEffectSparks::getGlowRadius() const
{
    return _glowRadius;
}

void BattleDialogModelEffectSparks::setGlowRadius(qreal radius)
{
    if(!qFuzzyCompare(_glowRadius, radius))
    {
        _glowRadius = radius;
        registerChange();
    }
}

qreal BattleDialogModelEffectSparks::getGlowOpacity() const
{
    return _glowOpacity;
}

void BattleDialogModelEffectSparks::setGlowOpacity(qreal opacity)
{
    if(!qFuzzyCompare(_glowOpacity, opacity))
    {
        _glowOpacity = opacity;
        registerChange();
    }
}

bool BattleDialogModelEffectSparks::getArcFalloff() const
{
    return _arcFalloff;
}

void BattleDialogModelEffectSparks::setArcFalloff(bool arcFalloff)
{
    if(_arcFalloff != arcFalloff)
    {
        _arcFalloff = arcFalloff;
        registerChange();
    }
}

bool BattleDialogModelEffectSparks::getFadeDistance() const
{
    return _fadeDistance;
}

void BattleDialogModelEffectSparks::setFadeDistance(bool fadeDistance)
{
    if(_fadeDistance != fadeDistance)
    {
        _fadeDistance = fadeDistance;
        registerChange();
    }
}

qreal BattleDialogModelEffectSparks::getSparkSpeed() const
{
    return _sparkSpeed;
}

void BattleDialogModelEffectSparks::setSparkSpeed(qreal speed)
{
    if(!qFuzzyCompare(_sparkSpeed, speed))
    {
        _sparkSpeed = speed;
        registerChange();
    }
}

qreal BattleDialogModelEffectSparks::getWindDirection() const
{
    return _windDirection;
}

void BattleDialogModelEffectSparks::setWindDirection(qreal windDirection)
{
    if(!qFuzzyCompare(_windDirection, windDirection))
    {
        _windDirection = windDirection;
        registerChange();
    }
}

qreal BattleDialogModelEffectSparks::getWindStrength() const
{
    return _windStrength;
}

void BattleDialogModelEffectSparks::setWindStrength(qreal windStrength)
{
    if(!qFuzzyCompare(_windStrength, windStrength))
    {
        _windStrength = windStrength;
        registerChange();
    }
}

void BattleDialogModelEffectSparks::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("particleCount", _particleCount);
    element.setAttribute("glowRadius", _glowRadius);
    element.setAttribute("glowOpacity", _glowOpacity);
    element.setAttribute("arcFalloff", static_cast<int>(_arcFalloff));
    element.setAttribute("fadeDistance", static_cast<int>(_fadeDistance));
    element.setAttribute("sparkSpeed", _sparkSpeed);
    element.setAttribute("windDirection", _windDirection);
    element.setAttribute("windStrength", _windStrength);

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}
