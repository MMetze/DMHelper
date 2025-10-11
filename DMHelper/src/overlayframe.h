#ifndef OVERLAYFRAME_H
#define OVERLAYFRAME_H

#include <QFrame>

namespace Ui {
class OverlayFrame;
}

class Overlay;

class OverlayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OverlayFrame(Overlay& overlay, QWidget *parent = nullptr);
    ~OverlayFrame();

private:
    Ui::OverlayFrame *ui;
    Overlay& _overlay;
};

#endif // OVERLAYFRAME_H
