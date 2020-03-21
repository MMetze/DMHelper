#ifndef RIBBONTABMAP_H
#define RIBBONTABMAP_H

#include <QFrame>

namespace Ui {
class RibbonTabMap;
}

class RibbonTabMap : public QFrame
{
    Q_OBJECT

public:
    explicit RibbonTabMap(QWidget *parent = nullptr);
    ~RibbonTabMap();

public slots:
    void setDistance(const QString& distance);

signals:
    void cameraCoupleClicked(bool checked);
    void cameraZoomClicked();
    void cameraSelectClicked(bool checked);
    void cameraEditClicked(bool checked);

    void distanceClicked(bool checked);
    void heightClicked(bool checked);
    void heightChanged(const QString& height);

    void gridClicked(bool checked);
    void gridScaleChanged(int scale);
    void gridXOffsetChanged(int offset);
    void gridYOffsetChanged(int offset);

    void editFoWClicked(bool checked);
    void drawEraseClicked(bool checked);
    void smoothClicked(bool checked);
    void brushCircleClicked();
    void brushSquareClicked();
    void brushSizeChanged(int size);
    void selectFoWClicked(bool checked);
    void fillFoWClicked();

    void pointerClicked(bool checked);

private:
    Ui::RibbonTabMap *ui;
};

#endif // RIBBONTABMAP_H
