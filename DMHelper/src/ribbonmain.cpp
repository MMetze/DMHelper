#include "ribbonmain.h"
#include "ribbonframe.h"
#include <QDebug>

RibbonMain::RibbonMain(QWidget *parent) :
    QTabWidget(parent),
   _publishProxy()
{
    connect(this, &QTabWidget::currentChanged, this, &RibbonMain::handleIndexChanged);
}

RibbonMain::~RibbonMain()
{
}

PublishButtonProxy* RibbonMain::getPublishRibbon()
{
    return &_publishProxy;
}

void RibbonMain::enableTab(RibbonFrame* page)
{
    if(indexOf(page) == -1)
    {
        addTab(page, page->windowTitle());
        if(page)
            _publishProxy.addPublishButton(page->getPublishRibbon());
    }

    setCurrentWidget(page);
}

void RibbonMain::disableTab(RibbonFrame* page)
{
    int pageIndex = indexOf(page);
    if(pageIndex != -1)
    {
        removeTab(pageIndex);
        if(page)
        {
            page->hide();
            _publishProxy.removePublishButton(page->getPublishRibbon());
        }
    }
}

void RibbonMain::handleIndexChanged(int index)
{
    qDebug() << "[RibbonMain] Ribbon index changed to " << index;
}
