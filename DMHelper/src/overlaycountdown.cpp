#include "overlaycountdown.h"
#include "dmconstants.h"

OverlayCountdown::OverlayCountdown(QObject *parent) :
    Overlay{parent}
{
}

int OverlayCountdown::getOverlayType() const
{
    return DMHelper::OverlayType_Counter;
}

