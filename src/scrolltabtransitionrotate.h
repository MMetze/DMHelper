#ifndef SCROLLTABTRANSITIONROTATE_H
#define SCROLLTABTRANSITIONROTATE_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>

class ScrollTabTransitionRotate : public ScrollTabTransitionBase
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionRotate(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = 0);

    virtual void paintTransition(QPainter& painter);

protected:
    virtual void handleStart();
    virtual void handleStep();

    QPixmap _widgetPmp;
};

#endif // SCROLLTABTRANSITIONROTATE_H
