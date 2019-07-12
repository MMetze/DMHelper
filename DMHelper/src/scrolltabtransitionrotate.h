#ifndef SCROLLTABTRANSITIONROTATE_H
#define SCROLLTABTRANSITIONROTATE_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>

class ScrollTabTransitionRotate : public ScrollTabTransitionBase
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionRotate(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);

    virtual void paintTransition(QPainter& painter) override;

protected:
    virtual void handleStart() override;
    virtual void handleStep() override;

    QPixmap _widgetPmp;
};

#endif // SCROLLTABTRANSITIONROTATE_H
