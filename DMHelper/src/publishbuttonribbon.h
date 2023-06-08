#ifndef PUBLISHBUTTONRIBBON_H
#define PUBLISHBUTTONRIBBON_H

#include "ribbonframe.h"
#include "layer.h"

namespace Ui {
class PublishButtonRibbon;
}

class PublishButtonRibbon : public RibbonFrame
{
    Q_OBJECT

public:
    explicit PublishButtonRibbon(QWidget *parent = nullptr);
    virtual ~PublishButtonRibbon();

    virtual PublishButtonRibbon* getPublishRibbon() override;

    virtual bool isChecked() const;
    virtual bool isCheckable() const;
    virtual int getRotation() const;
    virtual QColor getColor() const;

public slots:
    void setChecked(bool checked);
    void setCheckable(bool checkable);
    void setRotation(int rotation);
    void setColor(const QColor& color);
    void clickPublish();
    void cancelPublish();
    void setPlayersWindow(bool checked);
    void setLayersEnabled(bool enabled);
    void setLayers(QList<Layer*> layers, int selected);
    void updateLayerIcons();

signals:
    void clicked(bool checked = false);
    void layerSelected(int selected);
    void layersClicked();
    void rotateCW();
    void rotateCCW();
    void rotationChanged(int rotation);
    void colorChanged(const QColor& color);
    void buttonColorChanged(const QColor& color);
    void playersWindowClicked(bool checked);

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void handleToggle(bool checked);
    void handleClicked(bool checked);
    void handleRotation();
    void handleColorChanged(const QColor& color);
    void selectLayerAction(QAction* action);

private:
    void setDefaults();

    Ui::PublishButtonRibbon *ui;
};

#endif // PUBLISHBUTTONRIBBON_H
