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

protected:
    virtual void showEvent(QShowEvent *event) override;

    void setStandardButtonSize(QLabel& label, QPushButton& button);
    void setLineHeight(QFrame& line);
    void setLineHeight(QFrame& line, int fullHeight);
    int getLabelHeight(QLabel& label) const;
    int getLabelHeight(const QFontMetrics& metrics) const;
    void setWidgetSize(QWidget& widget, int w, int h) const;
    void setButtonSize(QPushButton& button, int w, int h) const;
    int getIconSize(int buttonWidth, int buttonHeight) const;
};

#endif // RIBBONFRAME_H
