#ifndef RIBBONTABBATTLEMAP_H
#define RIBBONTABBATTLEMAP_H

#include "ribbonframe.h"
#include <QAction>

namespace Ui {
class RibbonTabBattleMap;
}

class PublishButtonRibbon;
class QMenu;

class RibbonTabBattleMap : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabBattleMap(QWidget *parent = nullptr);
    ~RibbonTabBattleMap();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setGridOn(bool checked);
    void setGridType(int gridType);
    void setGridScale(int scale);
    void setGridAngle(int angle);
    void setGridXOffset(int offset);
    void setGridYOffset(int offset);

    void setEditFoW(bool checked);
    void setDrawErase(bool checked);
    void setSmooth(bool checked);
    void setBrushSize(int size);
    void setSelectFoW(bool checked);

signals:
    void newMapClicked();
    void reloadMapClicked();
    void gridClicked(bool checked);
    void gridTypeChanged(int gridType);
    void gridScaleChanged(int scale);
    void gridAngleChanged(int angle);
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

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

private slots:
    void setEraseMode();
    void spinChanged(int value);
    void selectAction(QAction* action);
    void setGridButtonIcon(const QIcon &icon);

private:
    Ui::RibbonTabBattleMap *ui;

    QMenu* _menu;
    int _timerId;
    int _lastGridScale;
    int _lastGridAngle;
};

class RibbonTabBattleMap_GridAction : public QAction
{
    Q_OBJECT

public:
    explicit RibbonTabBattleMap_GridAction(int gridType, const QString& actionIcon, const QString& actionText, QObject *parent = nullptr) :
        QAction(QIcon(actionIcon), actionText, parent),
        _gridType(gridType)
    {}

    virtual ~RibbonTabBattleMap_GridAction() override {}

    int getGridType() const { return _gridType; }

protected:
    int _gridType;

};



#endif // RIBBONTABBATTLEMAP_H
