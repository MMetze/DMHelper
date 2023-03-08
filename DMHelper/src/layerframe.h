#ifndef LAYERFRAME_H
#define LAYERFRAME_H

#include <QFrame>

namespace Ui {
class LayerFrame;
}

class Layer;

class LayerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LayerFrame(Layer& layer, QWidget *parent = nullptr);
    ~LayerFrame();

    void setLayerVisible(bool visible);
    void setIcon(const QImage& image);
    void setName(const QString& name);
    void setOpacity(int opacity);
    void setPosition(const QPoint& position);
    void setX(int x);
    void setY(int y);
    void setSize(const QSize& size);
    void setWidth(int width);
    void setHeight(int height);
    void setSelected(bool selected);

    const Layer& getLayer() const;
    Layer& getLayer();

signals:
    void visibleChanged(bool visible);
    void nameChanged(const QString& name);
    void opacityChanged(qreal opacity);
    void positionChanged(const QPoint& position);
    void sizeChanged(const QSize& size);

    void selectMe(LayerFrame* me);

protected slots:
    void handleNameChanged();
    void handleOpacityChanged();
    void handleXChanged();
    void handleYChanged();
    void handleWidthChanged();
    void handleHeightChanged();
    void handleLockClicked();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    QString getStyleString(bool selected);

    void updatePosition(int x, int y);
    void updateSize(int width, int height);

private:
    Ui::LayerFrame *ui;

    Layer& _layer;
    int _opacity;
};

#endif // LAYERFRAME_H
