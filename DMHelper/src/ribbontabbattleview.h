#ifndef RIBBONTABBATTLEVIEW_H
#define RIBBONTABBATTLEVIEW_H

#include "ribbonframe.h"
#include <QAction>

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

    bool getIsBattle() const;

public slots:
    void setIsBattle(bool isBattle);
    void setGridLocked(bool locked);

    void setZoomSelect(bool checked);
    void setCameraSelect(bool checked);
    void setCameraEdit(bool checked);

    void setDistanceOn(bool checked);
    void setFreeDistanceOn(bool checked);
    void setDistance(const QString& distance);
    void setDistanceScale(int scale);

    void setDistanceLineType(int lineType);
    void setDistanceLineColor(const QColor& color);
    void setDistanceLineWidth(int lineWidth);

    void setPointerOn(bool checked);
    void setPointerFile(const QString& filename);

signals:
    void zoomInClicked();
    void zoomOutClicked();
    void zoomFullClicked();
    void zoomSelectClicked(bool checked);

    void cameraCoupleClicked(bool checked);
    void cameraZoomClicked();
    void cameraVisibleClicked();
    void cameraSelectClicked(bool checked);
    void cameraEditClicked(bool checked);

    void distanceClicked(bool checked);
    void freeDistanceClicked(bool);
    void distanceScaleChanged(int scale);
    void heightChanged(bool checked, qreal height);

    void distanceLineTypeChanged(int lineType);
    void distanceLineColorChanged(const QColor& color);
    void distanceLineWidthChanged(int lineWidth);

    void pointerClicked(bool checked);

protected:
    virtual void showEvent(QShowEvent *event) override;

protected slots:
    void freeScaleEdited(const QString &text);
    void selectLineTypeAction(QAction* action);
    void heightEdited();

private:
    Ui::RibbonTabBattleView *ui;

    bool _isBattle;
};

class RibbonTabBattleView_LineTypeAction : public QAction
{
    Q_OBJECT

public:
    explicit RibbonTabBattleView_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent = nullptr);
    virtual ~RibbonTabBattleView_LineTypeAction() override;

    int getLineType() const;

protected:
    int _lineType;

};

#endif // RIBBONTABBATTLEVIEW_H
