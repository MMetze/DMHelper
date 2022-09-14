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
    void setSelected(bool selected);

    const Layer& getLayer() const;
    Layer& getLayer();

signals:
    void visibleChanged(bool visible);
    void nameChanged(const QString& name);
    void selectMe(LayerFrame* me);

protected slots:
    void handleNameChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    QString getStyleString(bool selected);

private:
    Ui::LayerFrame *ui;

    Layer& _layer;
};

#endif // LAYERFRAME_H
