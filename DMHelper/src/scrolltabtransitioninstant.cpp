#include "scrolltabtransitioninstant.h"
#include "scrolltabwidget.h"

ScrollTabTransitionInstant::ScrollTabTransitionInstant(ScrollTabWidget* tabWidget, bool deleteOnFinish, QObject *parent) :
    ScrollTabTransitionBase(tabWidget, deleteOnFinish, parent)
{
    setSteps(1);
    setDuration(1);
}

void ScrollTabTransitionInstant::paintTransition(QPainter& painter)
{
    Q_UNUSED(painter);

    return;
}
