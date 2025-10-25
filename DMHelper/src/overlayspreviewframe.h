#ifndef OVERLAYSPREVIEWFRAME_H
#define OVERLAYSPREVIEWFRAME_H

#include <QFrame>

namespace Ui {
class OverlaysPreviewFrame;
}

class Campaign;

class OverlaysPreviewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OverlaysPreviewFrame(QWidget *parent = nullptr);
    ~OverlaysPreviewFrame();

    void setCampaign(Campaign* campaign);

protected slots:
    void toggleOverlayFrame();
    void populateOverlays();
    void clearOverlays();

private:
    Ui::OverlaysPreviewFrame *ui;

    Campaign* _campaign;
};

#endif // OVERLAYSPREVIEWFRAME_H
