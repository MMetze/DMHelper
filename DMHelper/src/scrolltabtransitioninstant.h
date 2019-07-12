#ifndef SCROLLTABTRANSITIONINSTANT_H
#define SCROLLTABTRANSITIONINSTANT_H

#include "scrolltabtransitionbase.h"

class ScrollTabTransitionInstant : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionInstant(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);

    virtual void paintTransition(QPainter& painter) override;
};

#endif // SCROLLTABTRANSITIONINSTANT_H
