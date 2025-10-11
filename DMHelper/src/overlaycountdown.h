#ifndef OVERLAYCOUNTDOWN_H
#define OVERLAYCOUNTDOWN_H

#include <QObject>
#include "overlay.h"

class OverlayCountdown : public Overlay
{
    Q_OBJECT
public:
    explicit OverlayCountdown(QObject *parent = nullptr);

    virtual int getOverlayType() const override;

};

#endif // OVERLAYCOUNTDOWN_H
