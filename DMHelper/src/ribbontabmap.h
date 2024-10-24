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
    void setMapEdit(bool checked);
    void setBrushMode(int brushMode);
    void setDrawErase(bool checked);

signals:
    void editFileClicked();

    void mapEditClicked(bool checked);
    void drawEraseClicked(bool checked);
    void smoothClicked(bool checked);
    void brushModeChanged(int brushMode);
    void brushSizeChanged(int size);
    void fillFoWClicked();

    void colorizeClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void setEraseMode();

private:
    Ui::RibbonTabMap *ui;
};

#endif // RIBBONTABMAP_H
