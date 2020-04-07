#ifndef SCROLLOVERLAYWIDGET_H
#define SCROLLOVERLAYWIDGET_H

#include <QWidget>
#include <QList>
#include <QIcon>
#include "scrolltabwidget.h"
#include <QPushButton>
#include <QMap>

class QPushButton;
class ScrollTabWidget;
class QVBoxLayout;

class ScrollOverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollOverlayWidget(QWidget *parent = nullptr);
    virtual ~ScrollOverlayWidget();

signals:

public slots:
    virtual void repositionTab(ScrollTabWidget* tab);
    virtual void resizeTabs();
    virtual void setAnimatedTransitions(bool animated);

public:
    int addTab(ScrollTabWidget* tab, const QIcon &icon);

protected:
    virtual void moveEvent(QMoveEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

    virtual void repositionAllTabs();

    QMap<QPushButton*, ScrollTabWidget*> _tabList;
    QVBoxLayout* _mainLayout;
    QPoint _widgetOffset;
};

#endif // SCROLLOVERLAYWIDGET_H
