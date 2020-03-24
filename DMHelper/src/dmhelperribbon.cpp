#include "dmhelperribbon.h"
#include <QStylePainter>
#include <QStyleOptionTab>



DMHelperRibbon::DMHelperRibbon(QWidget *parent) :
    QTabWidget(parent)
{
    //setStyleSheet(QString("color: rgb(85, 85, 255); background-color: rgb(85, 85, 255);"));
    //setStyleSheet("QTabBar::tab {color: #000000;} QTabBar::tab:selected {background-color: #FA9944; color: #000000; border-top: #FA9944;} QTabBar::tab:hover {color: #000000; border-top: #FA9944; background-color: #FFFFFF;}");
    //setStyleSheet("QTabBar::tab {color: #000000; font-weight: bold; font-size: 10px; font-family: Gotham, Helvetica Neue, Helvetica, Arial, sans-serif;} QTabBar::tab:selected {background-color: #FA9944; color: #000000; border-top: 1px solid #FA9944;} QTabBar::tab:hover {color: #000000; border-top: 1px solid #FA9944; background-color: #FFFFFF;}");
}

DMHelperRibbon::~DMHelperRibbon()
{
}

void DMHelperRibbon::enableTab(QWidget* page)
{
    if(indexOf(page) == -1)
        addTab(page, page->windowTitle());

    setCurrentWidget(page);
}

void DMHelperRibbon::disableTab(QWidget* page)
{
    int pageIndex = indexOf(page);
    if(pageIndex != -1)
    {
        removeTab(pageIndex);
        page->hide();
    }
}
