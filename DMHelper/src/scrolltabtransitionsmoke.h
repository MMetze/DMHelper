#ifndef SCROLLTABTRANSITIONSMOKE_H
#define SCROLLTABTRANSITIONSMOKE_H

#include "scrolltabtransitionbase.h"
#include <QPixmap>
#include <QList>

class ScrollTabTransitionSmoke : public ScrollTabTransitionBase
{
public:
    explicit ScrollTabTransitionSmoke(ScrollTabWidget* tabWidget, bool deleteOnFinish = true, QObject *parent = nullptr);

    virtual void paintTransition(QPainter& painter) override;

protected:
    virtual void handleStart() override;
    virtual void handleStep() override;

    QPixmap _widgetPmp;
    QList<QPixmap> _animPmp;
};

#endif // SCROLLTABTRANSITIONSMOKE_H
