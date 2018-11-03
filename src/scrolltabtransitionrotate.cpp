#include "scrolltabtransitionrotate.h"
#include "scrolltabwidget.h"

ScrollTabTransitionRotate::ScrollTabTransitionRotate(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    ScrollTabTransitionBase(tabWidget, deleteOnFinish, parent),
    _widgetPmp()
{
    setDuration(300);
    setSteps(15);
}

void ScrollTabTransitionRotate::paintTransition(QPainter& painter)
{
    painter.drawPixmap(_widgetPmp.width() * (_steps - _currentStep) / _steps, 0, _widgetPmp);
}

void ScrollTabTransitionRotate::handleStart()
{
    _widgetPmp = _tabWidget->grab();
}

void ScrollTabTransitionRotate::handleStep()
{
    _tabWidget->repaint();
}
