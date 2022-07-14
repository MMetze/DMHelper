#ifndef CONFIGUREGRIDDIALOG_H
#define CONFIGUREGRIDDIALOG_H

#include <QDialog>

class BattleDialogModel;
class Grid;
class QGraphicsScene;

namespace Ui {
class ConfigureGridDialog;
}

class ConfigureGridDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureGridDialog(QWidget *parent = nullptr);
    ~ConfigureGridDialog();

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

    Ui::ConfigureGridDialog *ui;

    BattleDialogModel* _model;
    Grid* _grid;
    QGraphicsScene* _scene;
    qreal _gridScale;
};

#endif // CONFIGUREGRIDDIALOG_H
