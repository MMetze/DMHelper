#ifndef RIBBONTABMAP_H
#define RIBBONTABMAP_H

#include "dmconstants.h"
#include "ribbonframe.h"

namespace Ui {
class RibbonTabMap;
}

class PublishButtonRibbon;

class RibbonTabMap : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabMap(QWidget *parent = nullptr);
    ~RibbonTabMap();

    PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setZoomSelect(bool checked);
    void setBrushMode(int brushMode);
    void setDrawErase(bool checked);

signals:
    void editFileClicked();
    void zoomInClicked();
    void zoomOutClicked();
    void zoomOneClicked();
    void zoomFullClicked();
    void zoomSelectClicked(bool checked);

    void drawEraseClicked(bool checked);
    void smoothClicked(bool checked);
    void brushModeChanged(int brushMode);
    void brushSizeChanged(int size);
    void fillFoWClicked();

    void publishZoomChanged(bool checked);
    void publishVisibleChanged(bool checked);

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void setEraseMode();
    void publishModeVisibleClicked();
    void publishModeZoomClicked();

private:
    Ui::RibbonTabMap *ui;
};

#endif // RIBBONTABMAP_H
