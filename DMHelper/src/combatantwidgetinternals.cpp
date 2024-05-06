#include "combatantwidgetinternals.h"
#include "dmconstants.h"
#include "battledialogmodel.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>

CombatantWidgetInternals::CombatantWidgetInternals(QWidget *parent) :
    QObject(parent),
    _mouseDown(Qt::NoButton)
{
}

int CombatantWidgetInternals::getInitiative() const
{
    return 0;
}

bool CombatantWidgetInternals::isShown()
{
    return true;
}

bool CombatantWidgetInternals::isKnown()
{
    return true;
}

void CombatantWidgetInternals::leaveEvent(QEvent * event)
{
    Q_UNUSED(event);

    _mouseDown = Qt::NoButton;
}

void CombatantWidgetInternals::mousePressEvent(QMouseEvent * event)
{
    Q_UNUSED(event);

    _mouseDown = event->button();
}

void CombatantWidgetInternals::mouseReleaseEvent(QMouseEvent * event)
{
    if(_mouseDown == event->button())
    {
        if(event->button() == Qt::RightButton)
        {
            if(getCombatant())
            {
                emit contextMenu(getCombatant(), event->globalPosition().toPoint());
            }
        }
        _mouseDown = Qt::NoButton;
    }
}

void CombatantWidgetInternals::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(getFrame()->rect().contains(event->pos()))
            executeDoubleClick();
    }
}

void CombatantWidgetInternals::updateData()
{
}

void CombatantWidgetInternals::updateImage()
{
}

void CombatantWidgetInternals::initiativeChanged(int initiative)
{
    if(getCombatant() && (getCombatant()->getInitiative() != initiative))
    {
        getCombatant()->setInitiative(initiative);
        updateData();
    }
}

void CombatantWidgetInternals::moveChanged(int move)
{
    if(getCombatant())
        getCombatant()->setMoved(move);
}

void CombatantWidgetInternals::handleHitPointsChanged(int hp)
{
    if(getCombatant() && (getCombatant()->getHitPoints() != hp))
    {
        int hpDelta = hp - getCombatant()->getHitPoints();
        getCombatant()->setHitPoints(hp);
        updateData();
        emit hitPointsChanged(getCombatant(), hpDelta);
    }
}

void CombatantWidgetInternals::executeDoubleClick()
{
}
