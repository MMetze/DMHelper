#ifndef SCROLLTABTRANSITIONANIMATION_H
#define SCROLLTABTRANSITIONANIMATION_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionAnimation : public ScrollTabTransitionBase
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionAnimation(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);

    virtual void paintTransition(QPainter& painter) override;

protected:
    virtual void handleStart() override;
    virtual void handleStep() override;

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
    int _xPos;
};

#endif // SCROLLTABTRANSITIONANIMATION_H
