#ifndef MAPEDITFRAME_H
#define MAPEDITFRAME_H

#include "dmconstants.h"
#include <QFrame>

namespace Ui {
class MapEditFrame;
}

class MapEditFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MapEditFrame(DMHelper::BrushType brushType = DMHelper::BrushType_Circle,
                          int brushSize = 30,
                          bool gridVisible = true,
                          int gridScale = 25,
                          int gridOffsetX = 0,
                          int gridOffsetY = 0,
                          QWidget *parent = nullptr);
    ~MapEditFrame();

    DMHelper::BrushType getBrushType() const;
    int getBrushSize() const;

    bool isGridVisible() const;
    int getGridScale() const;
    int getGridOffsetX() const;
    int getGridOffsetY() const;

public slots:
    void setBrushType(DMHelper::BrushType brushType);
    void setBrushSize(int brushSize);

    void setGridVisible(bool gridVisible);
    void setGridScale(int gridScale);
    void setGridOffsetX(int gridOffsetX);
    void setGridOffsetY(int gridOffsetY);

signals:
    void clearFoW();
    void resetFoW();

    void brushTypeChanged(DMHelper::BrushType brushType);
    void brushSizeChanged(int brushSize);

    void gridVisibleChanged(bool gridVisible);
    void gridScaleChanged(int gridScale);
    void gridOffsetXChanged(int gridOffsetX);
    void gridOffsetYChanged(int gridOffsetY);

private:
    Ui::MapEditFrame *ui;
};

#endif // MAPEDITFRAME_H
