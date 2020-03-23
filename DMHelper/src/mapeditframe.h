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
    explicit MapEditFrame(QWidget *parent = nullptr);
    ~MapEditFrame();


public slots:
    void setZoomSelect(bool checked);
    void setBrushMode(int brushMode);
    void setDrawErase(bool checked);

signals:
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

private slots:
    void setEraseMode();
    void publishModeVisibleClicked();
    void publishModeZoomClicked();

private:
    Ui::MapEditFrame *ui;
};

#endif // MAPEDITFRAME_H
