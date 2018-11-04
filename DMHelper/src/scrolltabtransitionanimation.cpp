#include "scrolltabtransitionanimation.h"
#include "scrolltabwidget.h"

ScrollTabTransitionAnimation::ScrollTabTransitionAnimation(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    ScrollTabTransitionBase(tabWidget, deleteOnFinish, parent),
    _widgetPmp(),
    _animPmp(),
    _xPos(0)
{
    _animPmp.append(QPixmap(":/img/data/lightning/lightning01.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning02.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning03.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning05.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning06.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning07.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning08.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning09.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning10.png"));
    _animPmp.append(QPixmap(":/img/data/lightning/lightning11.png"));

    setSteps(11);
    setDuration(50 * 11);
}

void ScrollTabTransitionAnimation::paintTransition(QPainter& painter)
{
    if(_currentStep < 3)
    {
        QPixmap pmp = _animPmp.at(_currentStep);
        painter.drawPixmap(_xPos, 0, pmp.width(), _tabWidget->height(), pmp);
    }
    else if(_currentStep == 3)
    {
        painter.fillRect(_tabWidget->rect(), Qt::white);
    }
    else if(_currentStep < 11)
    {
        QPixmap pmp = _animPmp.at(_currentStep-1);
        painter.drawPixmap(0, 0, _widgetPmp);
        painter.drawPixmap(_xPos, 0, pmp.width(), _tabWidget->height(), pmp);
    }
    else
    {
        painter.drawPixmap(0, 0, _widgetPmp);
    }
}

void ScrollTabTransitionAnimation::handleStart()
{
    _widgetPmp = _tabWidget->grab();
    _xPos = (_widgetPmp.width() - _animPmp.first().width()) / 2;
}

void ScrollTabTransitionAnimation::handleStep()
{
    _tabWidget->repaint();
}
