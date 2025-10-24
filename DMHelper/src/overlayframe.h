#ifndef OVERLAYFRAME_H
#define OVERLAYFRAME_H

#include <QFrame>

namespace Ui {
class OverlayFrame;
}

class Overlay;
class QHBoxLayout;

class OverlayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OverlayFrame(Overlay* overlay, QWidget *parent = nullptr);
    ~OverlayFrame();

    Overlay* getOverlay() const;
    void setSelected(bool selected);

    QHBoxLayout* getLayout() const;
    static const int OVERLAY_FRAME_INSERT_POINT;

protected slots:
    void handleNameChanged();
    void handleOpacityChanged(int value);
    void handleScaleSliderChanged(int value);
    void handleScaleSpinChanged(qreal value);

    QString getStyleString(bool selected);

private:
    Ui::OverlayFrame *ui;
    Overlay* _overlay;
};

#endif // OVERLAYFRAME_H
