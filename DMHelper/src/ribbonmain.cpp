#include "ribbonmain.h"
#include "ribbonframe.h"
#include "publishbuttonribbon.h"
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
    if(!page)
        return;

    if(indexOf(page) == -1)
    {
        int index = addTab(page, page->windowTitle());
        if(index > 1)
        {
            // Set the publishbutton of the new page to match the state of the Campaign Tab publish button
            RibbonFrame* campaignFrame = dynamic_cast<RibbonFrame*>(widget(1));
            if(campaignFrame)
            {
                page->getPublishRibbon()->setColor(campaignFrame->getPublishRibbon()->getColor());
                page->getPublishRibbon()->setRotation(campaignFrame->getPublishRibbon()->getRotation());
            }
        }
        _publishProxy.addPublishButton(page->getPublishRibbon());
    }

    //setCurrentWidget(page);
}

void RibbonMain::disableTab(RibbonFrame* page)
{
    if(!page)
        return;

    int pageIndex = indexOf(page);
    if(pageIndex != -1)
    {
        removeTab(pageIndex);
        page->hide();
        _publishProxy.removePublishButton(page->getPublishRibbon());
    }
}

void RibbonMain::handleIndexChanged(int index)
{
    qDebug() << "[RibbonMain] Ribbon index changed to " << index;
}