#ifndef SCROLLTABTRANSITIONBASE_H
#define SCROLLTABTRANSITIONBASE_H

#include <QObject>
#include <QPainter>

class ScrollTabWidget;
class QTimer;

class ScrollTabTransitionBase : public QObject
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionBase(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);
    virtual ~ScrollTabTransitionBase();

    virtual ScrollTabWidget* scrollTabWidget() const;
    virtual void setScrollTabWidget(ScrollTabWidget* tabWidget);

    virtual bool isStarted() const;

    virtual int duration() const;
    virtual void setDuration(int newDuration);

    virtual int steps() const;
    virtual void setSteps(int newSteps);

    virtual int currentStep() const;

    virtual void paintTransition(QPainter& painter) = 0;

signals:
    void step(int count);
    void completed();
    void stopped();

public slots:
    virtual void start();
    virtual void stop();

protected slots:
    virtual void timerExpired();

protected:
    virtual void handleStart();
    virtual void handleStep();
    virtual void handleComplete();
    virtual void handleStop();

    ScrollTabWidget* _tabWidget;
    QTimer* _timer;
    int _duration;
    int _steps;
    int _currentStep;
    bool _deleteOnFinish;
};

#endif // SCROLLTABTRANSITIONBASE_H
