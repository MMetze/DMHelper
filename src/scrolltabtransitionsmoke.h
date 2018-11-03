#ifndef SCROLLTABTRANSITIONSMOKE_H
#define SCROLLTABTRANSITIONSMOKE_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionSmoke : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionSmoke(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = 0);

    virtual void paintTransition(QPainter& painter);

protected:
    virtual void handleStart();
    virtual void handleStep();

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
};

#endif // SCROLLTABTRANSITIONSMOKE_H
