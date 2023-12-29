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
    void setLayerVisibleDM(bool visible);
    void setLayerVisiblePlayer(bool visible);
    void setLinkedUp(bool linkUp);
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

    bool isLinkedUp() const;
    bool isLayerVisible() const;
    bool isLayerVisibleDM() const;
    bool isLayerVisiblePlayer() const;

signals:
    void visibleDMChanged(bool visible);
    void visiblePlayerChanged(bool visible);
    void linkedUpChanged(bool linkedUp);
    void nameChanged(const QString& name);
    void opacityChanged(qreal opacity);
    void positionChanged(const QPoint& position);
    void sizeChanged(const QSize& size);

    void linkedUp(LayerFrame* layerFrame);
    void visibilityChanged(LayerFrame* layerFrame);
    void dmVisibilityChanged(LayerFrame* layerFrame);

    void selectMe(LayerFrame* me);
    void refreshPlayer();

protected slots:
    void handleLinkUp(bool checked);
    void handleVisibleClicked(bool checked);
    void handleVisibleChanged();
    void handleNameChanged();
    void handleOpacityChanged();
    void handleXChanged();
    void handleYChanged();
    void handleWidthChanged();
    void handleHeightChanged();
    void handleLockClicked();

    void updateLayerData();

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
