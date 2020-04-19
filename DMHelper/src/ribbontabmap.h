#ifndef RIBBONTABMAP_H
#define RIBBONTABMAP_H

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

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setZoomSelect(bool checked);

    void setCameraCouple(bool checked);
    void setDistanceOn(bool checked);
    void setDistance(const QString& distance);
    void setCameraSelect(bool checked);
    void setCameraEdit(bool checked);

    void setGridOn(bool checked);
    void setGridScale(int scale);
    void setGridXOffset(int offset);
    void setGridYOffset(int offset);

    void setEditFoW(bool checked);
    void setDrawErase(bool checked);
    void setSmooth(bool checked);
    void setBrushSize(int size);
    void setSelectFoW(bool checked);

    void setPointerOn(bool checked);
    void setPointerFile(const QString& filename);

signals:
    void zoomInClicked();
    void zoomOutClicked();
    void zoomFullClicked();
    void zoomSelectClicked(bool checked);

    void cameraCoupleClicked(bool checked);
    void cameraZoomClicked();
    void cameraSelectClicked(bool checked);
    void cameraEditClicked(bool checked);

    void distanceClicked(bool checked);
    void heightChanged(bool checked, qreal height);

    void gridClicked(bool checked);
    void gridScaleChanged(int scale);
    void gridXOffsetChanged(int offset);
    void gridYOffsetChanged(int offset);

    void editFoWClicked(bool checked);
    void drawEraseClicked(bool checked);
    void smoothClicked(bool checked);
    void brushCircleClicked();
    void brushSquareClicked();
    void brushModeChanged(int brushMode);
    void brushSizeChanged(int size);
    void selectFoWClicked(bool checked);
    void fillFoWClicked();

    void pointerClicked(bool checked);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void setEraseMode();
    void heightEdited();

private:
    Ui::RibbonTabMap *ui;
};

#endif // RIBBONTABMAP_H
