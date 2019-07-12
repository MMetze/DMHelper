#include "scrolltabtransitionfactory.h"
#include "scrolltabtransitionbase.h"
#include "scrolltabtransitionrotate.h"
#include "scrolltabtransitionanimation.h"
#include "scrolltabtransitionsmoke.h"
#include "scrolltabtransitionzap.h"
#include "scrolltabtransitioninstant.h"
#include <QRandomGenerator>
#include <QtGlobal>

ScrollTabTransitionFactory::ScrollTabTransitionFactory(QObject *parent) :
    QObject(parent)
{
}

ScrollTabTransitionBase* ScrollTabTransitionFactory::createTransition(ScrollTabWidget* widget)
{
    int randomValue = QRandomGenerator::system()->bounded(4);//qrand() % 4;

    switch(randomValue)
    {
        case 0:
            return new ScrollTabTransitionSmoke(widget);
        case 1:
            return new ScrollTabTransitionAnimation(widget);
        case 2:
            return new ScrollTabTransitionZap(widget);
        default:
            return new ScrollTabTransitionRotate(widget);
    }
}

ScrollTabTransitionBase* ScrollTabTransitionFactory::createInstantTransition(ScrollTabWidget* widget)
{
    return new ScrollTabTransitionInstant(widget);
}
