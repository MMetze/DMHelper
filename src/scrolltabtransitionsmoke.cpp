#include "scrolltabtransitionsmoke.h"
#include "scrolltabwidget.h"

ScrollTabTransitionSmoke::ScrollTabTransitionSmoke(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    ScrollTabTransitionBase(tabWidget, deleteOnFinish, parent),
    _widgetPmp(),
    _animPmp()
{
    _animPmp.append(QPixmap(":/img/data/smoke/smoke01.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke02.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke03.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke04.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke05.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke06.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke07.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke08.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke09.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke10.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke11.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke12.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke13.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke14.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke15.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke16.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke17.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke18.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke19.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke20.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke21.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke22.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke23.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke24.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke25.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke26.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke27.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke28.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke29.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke30.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke31.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke32.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke33.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke34.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke35.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke36.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke37.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke38.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke39.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke40.png"));
    _animPmp.append(QPixmap(":/img/data/smoke/smoke41.png"));

    setSteps(41);
    setDuration(700);
}

void ScrollTabTransitionSmoke::paintTransition(QPainter& painter)
{
    if(_currentStep < 22)
    {
        painter.drawPixmap(_widgetPmp.rect(), _animPmp.at(_currentStep));
    }
    else if(_currentStep < 41)
    {
        painter.drawPixmap(0, 0, _widgetPmp);
        painter.drawPixmap(_widgetPmp.rect(), _animPmp.at(_currentStep));
    }
    else
    {
        painter.drawPixmap(0, 0, _widgetPmp);
    }
}

void ScrollTabTransitionSmoke::handleStart()
{
    _widgetPmp = _tabWidget->grab();
}

void ScrollTabTransitionSmoke::handleStep()
{
    _tabWidget->repaint();
}

