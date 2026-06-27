#include "battletokenhealthbar.h"
#include "battledialogmodelcombatant.h"
#include "rulehealth.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

static constexpr qreal BAR_HEIGHT_FRACTION = 0.10;        // bar height as fraction of token height
static constexpr qreal BAR_OVERHANG_FRACTION = 0.10;      // horizontal overhang past each token edge (fraction of token width)
static constexpr qreal BAR_GAP_FRACTION = 0.02;           // gap between token bottom and bar top (fraction of token height)
static constexpr qreal BAR_CORNER_FRACTION = 0.45;        // corner radius as fraction of bar height
static constexpr qreal BAR_SHADOW_OFFSET_FRACTION = 0.25; // shadow offset as fraction of bar height
static constexpr qreal BAR_BORDER_WIDTH_FRACTION = 0.12;  // border pen width as fraction of bar height
static constexpr qreal RADIANS_TO_DEGREES = 57.29577951308232;

static const QColor BAR_BACKGROUND_COLOR(180, 30, 30);
static const QColor BAR_FOREGROUND_COLOR(40, 200, 60);
static const QColor BAR_BORDER_COLOR(20, 20, 20);
static const QColor BAR_SHADOW_COLOR(0, 0, 0, 120);

static qreal getSceneRotationDegrees(const QTransform& transform)
{
    // Extract rotation from the transformed local X axis.
    return qAtan2(transform.m12(), transform.m11()) * RADIANS_TO_DEGREES;
}

BattleTokenHealthBar::BattleTokenHealthBar(BattleDialogModelCombatant* combatant, QGraphicsItem* parent) :
    QGraphicsObject(parent),
    _combatant(combatant)
{
}

BattleTokenHealthBar::~BattleTokenHealthBar()
{
}

QRectF BattleTokenHealthBar::boundingRect() const
{
    const QGraphicsItem* p = parentItem();
    if(!p)
        return QRectF();

    const QRectF parentBounds = p->boundingRect();
    const qreal barHeight = parentBounds.height() * BAR_HEIGHT_FRACTION;
    const qreal overhang = parentBounds.width() * BAR_OVERHANG_FRACTION;
    const qreal gap = parentBounds.height() * BAR_GAP_FRACTION;
    const qreal shadowOffset = barHeight * BAR_SHADOW_OFFSET_FRACTION;

    return QRectF(parentBounds.left() - overhang,
                  parentBounds.bottom() + gap,
                  parentBounds.width() + (2.0 * overhang) + shadowOffset,
                  barHeight + shadowOffset);
}

void BattleTokenHealthBar::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const QGraphicsItem* p = parentItem();
    if(!p)
        return;

    const QRectF parentBounds = p->boundingRect();
    const qreal barHeight = parentBounds.height() * BAR_HEIGHT_FRACTION;
    if(barHeight <= 0.0)
        return;

    const qreal overhang = parentBounds.width() * BAR_OVERHANG_FRACTION;
    const qreal gap = parentBounds.height() * BAR_GAP_FRACTION;
    const qreal shadowOffset = barHeight * BAR_SHADOW_OFFSET_FRACTION;
    const qreal corner = barHeight * BAR_CORNER_FRACTION;
    const qreal borderWidth = barHeight * BAR_BORDER_WIDTH_FRACTION;

    const QRectF barRect(parentBounds.left() - overhang,
                         parentBounds.bottom() + gap,
                         parentBounds.width() + (2.0 * overhang),
                         barHeight);

    qreal fraction = 0.0;
    RuleHealth* ruleHealth = RuleHealth::forCombatant(_combatant);
    if(ruleHealth)
        fraction = ruleHealth->getHealthFraction(_combatant);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    const qreal parentSceneRotation = getSceneRotationDegrees(p->sceneTransform());
    if(!qFuzzyIsNull(parentSceneRotation))
    {
        const QPointF tokenCenter = parentBounds.center();
        painter->translate(tokenCenter);
        painter->rotate(-parentSceneRotation);
        painter->translate(-tokenCenter);
    }

    // Shadow
    const QRectF shadowRect = barRect.translated(shadowOffset, shadowOffset);
    painter->setPen(Qt::NoPen);
    painter->setBrush(BAR_SHADOW_COLOR);
    painter->drawRoundedRect(shadowRect, corner, corner);

    // Background fill (red)
    painter->setBrush(BAR_BACKGROUND_COLOR);
    painter->drawRoundedRect(barRect, corner, corner);

    // Green foreground, clipped to the rounded bar shape
    if(fraction > 0.0)
    {
        QPainterPath clipPath;
        clipPath.addRoundedRect(barRect, corner, corner);
        painter->save();
        painter->setClipPath(clipPath);
        QRectF greenRect = barRect;
        greenRect.setWidth(barRect.width() * fraction);
        painter->setBrush(BAR_FOREGROUND_COLOR);
        painter->drawRect(greenRect);
        painter->restore();
    }

    // Border
    QPen borderPen(BAR_BORDER_COLOR, borderWidth);
    borderPen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(barRect, corner, corner);

    painter->restore();
}
