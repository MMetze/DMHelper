#ifndef SCROLLTABTRANSITIONINSTANT_H
#define SCROLLTABTRANSITIONINSTANT_H

#include "scrolltabtransitionbase.h"

class ScrollTabTransitionInstant : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionInstant(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = 0);

    virtual void paintTransition(QPainter& painter);
};

#endif // SCROLLTABTRANSITIONINSTANT_H
