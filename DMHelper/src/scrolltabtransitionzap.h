#ifndef SCROLLTABTRANSITIONZAP_H
#define SCROLLTABTRANSITIONZAP_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionZap : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionZap(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);

    virtual void paintTransition(QPainter& painter) override;

protected:
    virtual void handleStart() override;
    virtual void handleStep() override;

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
};

#endif // SCROLLTABTRANSITIONZAP_H
