#ifndef RIBBONTABBATTLEVIEW_H
#define RIBBONTABBATTLEVIEW_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabBattleView;
}

class PublishButtonRibbon;

class RibbonTabBattleView : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBattleView(QWidget *parent = nullptr);
    ~RibbonTabBattleView();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setZoomSelect(bool checked);
    void setCameraSelect(bool checked);
    void setCameraEdit(bool checked);

    void setDistanceOn(bool checked);
    void setDistance(const QString& distance);

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

    void pointerClicked(bool checked);

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void heightEdited();

private:
    Ui::RibbonTabBattleView *ui;
};

#endif // RIBBONTABBATTLEVIEW_H
