#ifndef LAYERSEDITDIALOG_H
#define LAYERSEDITDIALOG_H

#include <QDialog>

namespace Ui {
class LayersEditDialog;
}

class LayerFrame;
class LayerScene;
class BattleDialogModel;
class QVBoxLayout;

class LayersEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayersEditDialog(LayerScene& scene, BattleDialogModel* model = nullptr, QWidget *parent = nullptr);
    ~LayersEditDialog();

protected slots:
    void selectFrame(LayerFrame* frame);
    void moveUp();
    void moveDown();
    void addLayer();
    void removeLayer();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void resetLayout();
    void readScene();
    void clearLayout();

    Ui::LayersEditDialog *ui;
    QVBoxLayout* _layerLayout;
    LayerScene& _scene;
    BattleDialogModel* _model;
};

#endif // LAYERSEDITDIALOG_H
