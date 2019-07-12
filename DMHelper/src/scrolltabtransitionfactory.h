#ifndef SCROLLTABTRANSITIONFACTORY_H
#define SCROLLTABTRANSITIONFACTORY_H

#include <QObject>

class ScrollTabTransitionBase;
class ScrollTabWidget;

class ScrollTabTransitionFactory : public QObject
{
    Q_OBJECT
public:
    explicit ScrollTabTransitionFactory(QObject *parent = nullptr);

    static ScrollTabTransitionBase* createTransition(ScrollTabWidget* widget);
    static ScrollTabTransitionBase* createInstantTransition(ScrollTabWidget* widget);

signals:

public slots:
};

#endif // SCROLLTABTRANSITIONFACTORY_H
