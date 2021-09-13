#ifndef RIBBONTABSCROLLING_H
#define RIBBONTABSCROLLING_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabScrolling;
}

class RibbonTabScrolling : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabScrolling(QWidget *parent = nullptr);
    ~RibbonTabScrolling();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    // Animation
    void setSpeed(int speed);
    void setWidth(int width);

    // Text Frame
    void setColor(const QColor& color);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setAlignment(Qt::Alignment alignment);

signals:
    // Animation
    void backgroundClicked();
    void speedChanged(int speed);
    void widthChanged(int width);
    void rewindClicked();

    // Text Frame
    void colorChanged(const QColor& color);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabScrolling *ui;
};

#endif // RIBBONTABSCROLLING_H
