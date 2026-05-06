#include "battletokenhealthbar.h"
#include "battledialogmodelcombatant.h"
#include "rulehealth.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

static constexpr qreal BAR_HEIGHT_FRACTION = 0.08;

BattleTokenHealthBar::BattleTokenHealthBar(BattleDialogModelCombatant* combatant, QGraphicsItem* parent)
    : QGraphicsObject(parent),
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
    return QRectF(parentBounds.left(), parentBounds.top() - barHeight, parentBounds.width(), barHeight);
}

void BattleTokenHealthBar::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const QRectF bounds = boundingRect();
    if(bounds.isEmpty())
        return;

    // Red background
    painter->fillRect(bounds, Qt::red);

    // Green foreground sized by health fraction
    qreal fraction = 0.0;
    RuleHealth* ruleHealth = RuleHealth::forCombatant(_combatant);
    if(ruleHealth)
        fraction = ruleHealth->getHealthFraction(_combatant);

    QRectF greenRect = bounds;
    greenRect.setWidth(bounds.width() * fraction);
    if(!greenRect.isEmpty())
        painter->fillRect(greenRect, Qt::green);
}
