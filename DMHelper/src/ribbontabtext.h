#ifndef RIBBONTABTEXT_H
#define RIBBONTABTEXT_H

#include "ribbonframe.h"

namespace Ui {
class RibbonTabText;
}

class RibbonTabText : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonTabText(QWidget *parent = nullptr);
    virtual ~RibbonTabText() override;

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setAnimation(bool checked);
    void setImageFile(const QString& imageFile);

    void setColor(QColor color);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setAlignment(Qt::Alignment alignment);

    void setWidth(int width);
    void setSpeed(int speed);

    void setHyperlinkActive(bool active);

signals:
    void backgroundClicked(bool checked);

    // Animation
    void animationClicked(bool checked);
    void speedChanged(int speed);
    void widthChanged(int width);
    void rewindClicked();

    // Text
    void colorChanged(QColor color);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void hyperlinkClicked();

    // Tools
    void translateTextClicked();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabText *ui;
};

#endif // RIBBONTABTEXT_H
