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
    void setColor(QColor color);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setAlignment(Qt::Alignment alignment);

signals:
    void colorChanged(QColor color);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);

private:
    Ui::RibbonTabText *ui;
};

#endif // RIBBONTABTEXT_H
