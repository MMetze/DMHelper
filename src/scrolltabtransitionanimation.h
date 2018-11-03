#ifndef SCROLLTABTRANSITIONANIMATION_H
#define SCROLLTABTRANSITIONANIMATION_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionAnimation : public ScrollTabTransitionBase
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionAnimation(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = 0);

    virtual void paintTransition(QPainter& painter);

protected:
    virtual void handleStart();
    virtual void handleStep();

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
    int _xPos;
};

#endif // SCROLLTABTRANSITIONANIMATION_H
