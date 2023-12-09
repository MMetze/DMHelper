#ifndef CONFIGURELOCKEDGRIDDIALOG_H
#define CONFIGURELOCKEDGRIDDIALOG_H

#include <QDialog>

class BattleDialogModel;
class Grid;
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

private:
    void rebuildGrid();
    qreal getPixelsPerInch();

    Ui::ConfigureLockedGridDialog *ui;

    BattleDialogModel* _model;
    Grid* _grid;
    QGraphicsScene* _scene;
    qreal _gridScale;
};

#endif // CONFIGURELOCKEDGRIDDIALOG_H
