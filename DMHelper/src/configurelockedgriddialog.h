#ifndef CONFIGURELOCKEDGRIDDIALOG_H
#define CONFIGURELOCKEDGRIDDIALOG_H

#include <QDialog>

class Grid;
class GridConfig;
class GridSizer;
class QGraphicsScene;

namespace Ui {
class ConfigureLockedGridDialog;
}

class ConfigureLockedGridDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureLockedGridDialog(QWidget *parent = nullptr);
    ~ConfigureLockedGridDialog();

    qreal getGridScale();

public slots:
    void setGridScale(qreal gridScale);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void toggleFullscreen();
    void gridScaleChanged(int value);
    void autoFit();
    void gridSizerResized();

private:
    void rebuildGrid();
    qreal getPixelsPerInch();

    Ui::ConfigureLockedGridDialog *ui;

    Grid* _grid;
    GridConfig* _gridConfig;
    GridSizer* _gridSizer;
    QGraphicsScene* _scene;
    qreal _gridScale;
};

#endif // CONFIGURELOCKEDGRIDDIALOG_H
