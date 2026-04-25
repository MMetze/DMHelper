#include "battledialogmodeleffectsmoke.h"
#include "battledialogeffectsettingssmoke.h"
#include "unselectedpixmap.h"
#include <QDomElement>
#include <QPainter>
#include <QRadialGradient>
#include <QtMath>

static constexpr qreal DEFAULT_DENSITY = 0.5;
static const QColor DEFAULT_CENTER_COLOR(80, 80, 80, 180);
static const QColor DEFAULT_EDGE_COLOR(200, 200, 200, 60);
static constexpr qreal DEFAULT_BILLOW_FACTOR = 0.5;
static constexpr qreal DEFAULT_WIND_DIRECTION = 0.0;
static constexpr qreal DEFAULT_WIND_STRENGTH = 0.0;
static constexpr int PREVIEW_PIXMAP_SIZE = 200;
static constexpr qreal CHANGE_EPSILON = 0.0001;

BattleDialogModelEffectSmoke::BattleDialogModelEffectSmoke(const QString& name, QObject *parent) :
    BattleDialogModelEffect(name, parent),
    _density(DEFAULT_DENSITY),
    _centerColor(DEFAULT_CENTER_COLOR),
    _edgeColor(DEFAULT_EDGE_COLOR),
    _billowFactor(DEFAULT_BILLOW_FACTOR),
    _windDirection(DEFAULT_WIND_DIRECTION),
    _windStrength(DEFAULT_WIND_STRENGTH)
{
    _active = false;
}

BattleDialogModelEffectSmoke::BattleDialogModelEffectSmoke(int size, const QPointF& position, qreal rotation, const QColor& color, const QString& tip) :
    BattleDialogModelEffect(size, position, rotation, color, tip),
    _density(DEFAULT_DENSITY),
    _centerColor(DEFAULT_CENTER_COLOR),
    _edgeColor(DEFAULT_EDGE_COLOR),
    _billowFactor(DEFAULT_BILLOW_FACTOR),
    _windDirection(DEFAULT_WIND_DIRECTION),
    _windStrength(DEFAULT_WIND_STRENGTH)
{
    _active = false;
}

BattleDialogModelEffectSmoke::~BattleDialogModelEffectSmoke()
{
}

void BattleDialogModelEffectSmoke::inputXML(const QDomElement &element, bool isImport)
{
    _density = element.attribute("density", QString::number(DEFAULT_DENSITY)).toDouble();
    _centerColor = QColor(element.attribute("centerColorR", QString::number(DEFAULT_CENTER_COLOR.red())).toInt(),
                          element.attribute("centerColorG", QString::number(DEFAULT_CENTER_COLOR.green())).toInt(),
                          element.attribute("centerColorB", QString::number(DEFAULT_CENTER_COLOR.blue())).toInt(),
                          element.attribute("centerColorA", QString::number(DEFAULT_CENTER_COLOR.alpha())).toInt());
    _edgeColor = QColor(element.attribute("edgeColorR", QString::number(DEFAULT_EDGE_COLOR.red())).toInt(),
                        element.attribute("edgeColorG", QString::number(DEFAULT_EDGE_COLOR.green())).toInt(),
                        element.attribute("edgeColorB", QString::number(DEFAULT_EDGE_COLOR.blue())).toInt(),
                        element.attribute("edgeColorA", QString::number(DEFAULT_EDGE_COLOR.alpha())).toInt());
    _billowFactor = element.attribute("billowFactor", QString::number(DEFAULT_BILLOW_FACTOR)).toDouble();
    _windDirection = element.attribute("windDirection", QString::number(DEFAULT_WIND_DIRECTION)).toDouble();
    _windStrength = element.attribute("windStrength", QString::number(DEFAULT_WIND_STRENGTH)).toDouble();

    BattleDialogModelEffect::inputXML(element, isImport);
}

void BattleDialogModelEffectSmoke::copyValues(const CampaignObjectBase* other)
{
    const BattleDialogModelEffectSmoke* otherEffect = dynamic_cast<const BattleDialogModelEffectSmoke*>(other);
    if(!otherEffect)
        return;

    _density = otherEffect->_density;
    _centerColor = otherEffect->_centerColor;
    _edgeColor = otherEffect->_edgeColor;
    _billowFactor = otherEffect->_billowFactor;
    _windDirection = otherEffect->_windDirection;
    _windStrength = otherEffect->_windStrength;

    BattleDialogModelEffect::copyValues(other);
}

QString BattleDialogModelEffectSmoke::getName() const
{
    return _tip.isEmpty() ? QString("Smoke Effect") : _tip;
}

BattleDialogModelEffect* BattleDialogModelEffectSmoke::clone() const
{
    BattleDialogModelEffectSmoke* newEffect = new BattleDialogModelEffectSmoke(getName());
    newEffect->copyValues(this);
    return newEffect;
}

int BattleDialogModelEffectSmoke::getEffectType() const
{
    return BattleDialogModelEffect_Smoke;
}

BattleDialogEffectSettingsBase* BattleDialogModelEffectSmoke::getEffectEditor() const
{
    return new BattleDialogEffectSettingsSmoke(*this);
}

QGraphicsItem* BattleDialogModelEffectSmoke::createEffectShape(qreal gridScale)
{
    int previewSize = PREVIEW_PIXMAP_SIZE;
    QPixmap pixmap(previewSize, previewSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient(previewSize / 2.0, previewSize / 2.0, previewSize / 2.0);
    gradient.setColorAt(0.0, _centerColor);
    gradient.setColorAt(1.0, _edgeColor);
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
    pixmapItem->setOpacity(_color.alphaF());

    return pixmapItem;
}

qreal BattleDialogModelEffectSmoke::getDensity() const
{
    return _density;
}

void BattleDialogModelEffectSmoke::setDensity(qreal density)
{
    if(qAbs(_density - density) > CHANGE_EPSILON)
    {
        _density = density;
        registerChange();
    }
}

QColor BattleDialogModelEffectSmoke::getCenterColor() const
{
    return _centerColor;
}

void BattleDialogModelEffectSmoke::setCenterColor(const QColor& color)
{
    if(_centerColor != color)
    {
        _centerColor = color;
        registerChange();
    }
}

QColor BattleDialogModelEffectSmoke::getEdgeColor() const
{
    return _edgeColor;
}

void BattleDialogModelEffectSmoke::setEdgeColor(const QColor& color)
{
    if(_edgeColor != color)
    {
        _edgeColor = color;
        registerChange();
    }
}

qreal BattleDialogModelEffectSmoke::getBillowFactor() const
{
    return _billowFactor;
}

void BattleDialogModelEffectSmoke::setBillowFactor(qreal billowFactor)
{
    if(qAbs(_billowFactor - billowFactor) > CHANGE_EPSILON)
    {
        _billowFactor = billowFactor;
        registerChange();
    }
}

qreal BattleDialogModelEffectSmoke::getWindDirection() const
{
    return _windDirection;
}

void BattleDialogModelEffectSmoke::setWindDirection(qreal windDirection)
{
    if(qAbs(_windDirection - windDirection) > CHANGE_EPSILON)
    {
        _windDirection = windDirection;
        registerChange();
    }
}

qreal BattleDialogModelEffectSmoke::getWindStrength() const
{
    return _windStrength;
}

void BattleDialogModelEffectSmoke::setWindStrength(qreal windStrength)
{
    if(qAbs(_windStrength - windStrength) > CHANGE_EPSILON)
    {
        _windStrength = windStrength;
        registerChange();
    }
}

void BattleDialogModelEffectSmoke::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("density", _density);
    element.setAttribute("centerColorR", _centerColor.red());
    element.setAttribute("centerColorG", _centerColor.green());
    element.setAttribute("centerColorB", _centerColor.blue());
    element.setAttribute("centerColorA", _centerColor.alpha());
    element.setAttribute("edgeColorR", _edgeColor.red());
    element.setAttribute("edgeColorG", _edgeColor.green());
    element.setAttribute("edgeColorB", _edgeColor.blue());
    element.setAttribute("edgeColorA", _edgeColor.alpha());
    element.setAttribute("billowFactor", _billowFactor);
    element.setAttribute("windDirection", _windDirection);
    element.setAttribute("windStrength", _windStrength);

    BattleDialogModelEffect::internalOutputXML(doc, element, targetDirectory, isExport);
}
