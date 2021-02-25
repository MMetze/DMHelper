#ifndef RIBBONFRAMETEXT_H
#define RIBBONFRAMETEXT_H

#include "ribbonframe.h"

namespace Ui {
class RibbonFrameText;
}

class RibbonFrameText : public RibbonFrame
{
    Q_OBJECT

public:
    explicit RibbonFrameText(QWidget *parent = nullptr);
    ~RibbonFrameText();

    virtual PublishButtonRibbon* getPublishRibbon() override;

public slots:
    void setColor(QColor color);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setFontUnderline(bool fontUnderline);
    void setAlignment(Qt::Alignment alignment);

signals:
    void colorChanged(QColor color);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void fontUnderlineChanged(bool fontUnderline);
    void alignmentChanged(Qt::Alignment alignment);

protected slots:
    virtual void handleFontSizeChanged();
    virtual void handleAlignmentChanged(int id);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    Ui::RibbonFrameText *ui;
};

#endif // RIBBONFRAMETEXT_H
