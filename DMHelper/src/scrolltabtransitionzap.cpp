#include "scrolltabtransitionzap.h"
#include "scrolltabwidget.h"

ScrollTabTransitionZap::ScrollTabTransitionZap(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    ScrollTabTransitionBase(tabWidget, deleteOnFinish, parent),
    _widgetPmp(),
    _animPmp()
{
    _animPmp.append(QPixmap(":/img/data/zap/zap01.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap02.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap03.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap04.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap05.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap06.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap07.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap08.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap09.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap10.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap11.png"));
    _animPmp.append(QPixmap(":/img/data/zap/zap12.png"));

    setSteps(12);
    setDuration(300);
}

void ScrollTabTransitionZap::paintTransition(QPainter& painter)
{
    if(_currentStep < 6)
    {
        painter.drawPixmap(_widgetPmp.rect(), _animPmp.at(_currentStep));
    }
    else if(_currentStep < 12)
    {
        painter.drawPixmap(0, 0, _widgetPmp);
        painter.drawPixmap(_widgetPmp.rect(), _animPmp.at(_currentStep));
    }
    else
    {
        painter.drawPixmap(0, 0, _widgetPmp);
    }
}

void ScrollTabTransitionZap::handleStart()
{
    _widgetPmp = _tabWidget->grab();
}

void ScrollTabTransitionZap::handleStep()
{
    _tabWidget->repaint();
}

