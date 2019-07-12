#include "scrolltabtransitionbase.h"
#include "scrolltabwidget.h"
#include <QTimer>

ScrollTabTransitionBase::ScrollTabTransitionBase(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    QObject(parent),
    _tabWidget(tabWidget),
    _timer(nullptr),
    _duration(1000),
    _steps(1),
    _currentStep(0),
    _deleteOnFinish(deleteOnFinish)
{
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(timerExpired()));
}

ScrollTabTransitionBase::~ScrollTabTransitionBase()
{
    stop();
}

ScrollTabWidget* ScrollTabTransitionBase::scrollTabWidget() const
{
    return _tabWidget;
}

void ScrollTabTransitionBase::setScrollTabWidget(ScrollTabWidget* tabWidget)
{
    _tabWidget = tabWidget;
}

bool ScrollTabTransitionBase::isStarted() const
{
    return (_timer->isActive());
}

int ScrollTabTransitionBase::duration() const
{
    return _duration;
}

void ScrollTabTransitionBase::setDuration(int newDuration)
{
    if(newDuration > 0)
        _duration = newDuration;
}

int ScrollTabTransitionBase::steps() const
{
    return _steps;
}

void ScrollTabTransitionBase::setSteps(int newSteps)
{
    if(newSteps > 0)
        _steps = newSteps;
}

int ScrollTabTransitionBase::currentStep() const
{
    return _currentStep;
}

void ScrollTabTransitionBase::start()
{
    stop();
    _currentStep = 0;
    handleStart();
    _timer->start(_duration / _steps);
}

void ScrollTabTransitionBase::stop()
{
    if(isStarted())
    {
        _timer->stop();
        handleStop();
        emit stopped();
    }
}

void ScrollTabTransitionBase::timerExpired()
{
    ++_currentStep;
    handleStep();
    emit step(currentStep());

    if(_currentStep >= _steps)
    {
        _timer->stop();
        handleComplete();
        emit completed();
        if(_deleteOnFinish)
            deleteLater();
    }
}

void ScrollTabTransitionBase::handleStart()
{
}

void ScrollTabTransitionBase::handleStep()
{
}

void ScrollTabTransitionBase::handleComplete()
{
}

void ScrollTabTransitionBase::handleStop()
{
}
