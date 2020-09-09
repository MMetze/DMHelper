#ifndef RIBBONFRAME_H
#define RIBBONFRAME_H

#include <QFrame>

class QLabel;
class QPushButton;
class PublishButtonRibbon;

class RibbonFrame : public QFrame
{
    Q_OBJECT
public:
    explicit RibbonFrame(QWidget *parent = nullptr);
    virtual ~RibbonFrame() override;

    virtual PublishButtonRibbon* getPublishRibbon() = 0;

    static void setStandardButtonSize(QLabel& label, QPushButton& button, int frameHeight);
    static void setLineHeight(QFrame& line, int frameHeight);
    static int getRibbonHeight(int defaultHeight = 0);
    static int getLabelHeight(QLabel& label, int frameHeight);
    static int getLabelHeight(const QFontMetrics& metrics, int frameHeight);
    static void setWidgetSize(QWidget& widget, int w, int h);
    static void setButtonSize(QPushButton& button, int w, int h);
    static int getIconSize(int buttonWidth, int buttonHeight);

protected:
    virtual void showEvent(QShowEvent *event) override;

};

#endif // RIBBONFRAME_H
