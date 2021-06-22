#include "widgetcombatantbase.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>

WidgetCombatantBase::WidgetCombatantBase(QWidget *parent) :
    QObject(parent),
    _mouseDown(Qt::NoButton)
{
}

int WidgetCombatantBase::getInitiative() const
{
    return 0;
}

bool WidgetCombatantBase::isShown()
{
    return true;
}

bool WidgetCombatantBase::isKnown()
{
    return true;
}

void WidgetCombatantBase::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);

    _mouseDown = Qt::NoButton;
}

void WidgetCombatantBase::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    _mouseDown = event->button();
}

void WidgetCombatantBase::mouseReleaseEvent(QMouseEvent * event)
{
    if(_mouseDown == event->button())
    {
        if(event->button() == Qt::RightButton)
        {
            if(getCombatant())
            {
                emit contextMenu(getCombatant(), event->globalPos());
            }
        }
        _mouseDown = Qt::NoButton;
    }
}

void WidgetCombatantBase::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(getFrame()->rect().contains(event->pos()))
            executeDoubleClick();
    }
}

void WidgetCombatantBase::updateData()
{
}

void WidgetCombatantBase::initiativeChanged(int initiative)
{
    if(getCombatant() && (getCombatant()->getInitiative() != initiative))
    {
        getCombatant()->setInitiative(initiative);
        updateData();
    }
}

void WidgetCombatantBase::moveChanged(int move)
{
    if(getCombatant())
        getCombatant()->setMoved(move);
}

void WidgetCombatantBase::handleHitPointsChanged(int hp)
{
    if(getCombatant() && (getCombatant()->getHitPoints() != hp))
    {
        int hpDelta = hp - getCombatant()->getHitPoints();
        getCombatant()->setHitPoints(hp);
        updateData();
        emit hitPointsChanged(getCombatant(), hpDelta);
    }
}

void WidgetCombatantBase::executeDoubleClick()
{
}
