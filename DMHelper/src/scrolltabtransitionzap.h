#ifndef SCROLLTABTRANSITIONZAP_H
#define SCROLLTABTRANSITIONZAP_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionZap : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionZap(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = 0);

    virtual void paintTransition(QPainter& painter);

protected:
    virtual void handleStart();
    virtual void handleStep();

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
};

#endif // SCROLLTABTRANSITIONZAP_H
