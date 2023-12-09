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

    void setColor(const QColor& color);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setFontUnderline(bool fontUnderline);
    void setAlignment(Qt::Alignment alignment);
    void setPasteRich(bool pasteRich);

    void setWidth(int width);
    void setSpeed(int speed);

    void setPlaying(bool playing);

    void setHyperlinkActive(bool active);

    void setTranslationActive(bool active);
    void setCodeView(bool active);
    void showCodeView(bool visible);

signals:
    // Animation
    void animationClicked(bool checked);
    void speedChanged(int speed);
    void widthChanged(int width);
    void rewindClicked();
    void playPauseClicked(bool playing);

    // Text
    void colorChanged(const QColor& color);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void fontUnderlineChanged(bool fontUnderline);
    void alignmentChanged(Qt::Alignment alignment);
    void pasteRichChanged(bool pasteRich);
    void hyperlinkClicked();

    // Tools
    void translateTextClicked(bool checked);
    void codeViewClicked(bool checked);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonTabText *ui;
};

#endif // RIBBONTABTEXT_H
