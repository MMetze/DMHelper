#ifndef PUBLISHBUTTONRIBBON_H
#define PUBLISHBUTTONRIBBON_H

#include "ribbonframe.h"

namespace Ui {
class PublishButtonRibbon;
}

class PublishButtonRibbon : public RibbonFrame
{
    Q_OBJECT

public:
    explicit PublishButtonRibbon(QWidget *parent = nullptr);
    virtual ~PublishButtonRibbon();

    /*
    bool isChecked();
    bool isCheckable();
    QColor getColor() const;
    int getRotation();
    */

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

signals:
    void clicked(bool checked = false);
//    void toggled(bool checked);
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

private:
    void setDefaults();

    Ui::PublishButtonRibbon *ui;
};

#endif // PUBLISHBUTTONRIBBON_H
