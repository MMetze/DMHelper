#ifndef RIBBONMAIN_H
#define RIBBONMAIN_H

#include <QTabWidget>
#include "publishbuttonproxy.h"

class RibbonFrame;
class PublishButtonRibbon;

class RibbonMain : public QTabWidget
{
    Q_OBJECT

public:
    explicit RibbonMain(QWidget *parent = nullptr);
    ~RibbonMain();

    PublishButtonProxy* getPublishRibbon();

public slots:
    void enableTab(RibbonFrame* page);
    void disableTab(RibbonFrame* page);

    void clickPublish();

private slots:
    void handleIndexChanged(int index);

private:
    PublishButtonProxy _publishProxy;
};

#endif // RIBBONMAIN_H
