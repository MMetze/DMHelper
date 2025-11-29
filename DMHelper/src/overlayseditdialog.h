#ifndef OVERLAYSEDITDIALOG_H
#define OVERLAYSEDITDIALOG_H

#include <QDialog>

namespace Ui {
class OverlaysEditDialog;
}

class Campaign;
class OverlayFrame;
class Overlay;
class QVBoxLayout;

class OverlaysEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverlaysEditDialog(Campaign& campaign, QWidget *parent = nullptr);
    ~OverlaysEditDialog();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    void selectFrame(OverlayFrame* frame);

    void handleNewOverlay();
    void handleDeleteOverlay();
    void handleMoveOverlayUp();
    void handleMoveOverlayDown();

    void resetLayout();
    void readOverlays();
    OverlayFrame* addOverlayFrame(Overlay* overlay);
    void clearLayout();

private:
    Ui::OverlaysEditDialog *ui;
    QVBoxLayout* _overlayLayout;
    Campaign& _campaign;
    OverlayFrame* _selectedFrame;
};

#endif // OVERLAYSEDITDIALOG_H
