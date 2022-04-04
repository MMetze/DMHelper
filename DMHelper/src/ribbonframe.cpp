#include "ribbonframe.h"
#include <QLabel>
#include <QAbstractButton>
#include <QGuiApplication>
#include <QScreen>

RibbonFrame::RibbonFrame(QWidget *parent) :
    QFrame(parent)
{
}

RibbonFrame::~RibbonFrame()
{
}

void RibbonFrame::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    int ribbonHeight = getRibbonHeight();
    if(ribbonHeight > 0)
    {
        setMinimumHeight(ribbonHeight);
        setMaximumHeight(ribbonHeight);
        resize(width(), ribbonHeight);
    }
}

void RibbonFrame::setStandardButtonSize(QLabel& label, QAbstractButton& button, int frameHeight)
{
    QFontMetrics metrics = label.fontMetrics();
    int labelHeight = getLabelHeight(metrics, frameHeight);
    int iconDim = frameHeight - labelHeight;
    int newWidth = qMax(metrics.horizontalAdvance(label.text()), iconDim);

    setWidgetSize(label, newWidth, labelHeight);
    setButtonSize(button, newWidth, iconDim);
}

void RibbonFrame::setButtonSizeOnly(QLabel& label, QAbstractButton& button, int frameHeight)
{
    QFontMetrics metrics = label.fontMetrics();
    int labelHeight = getLabelHeight(metrics, frameHeight);
    int iconWidth = frameHeight - labelHeight;

    setButtonSize(button, iconWidth, iconWidth);
}

void RibbonFrame::setLabelHeight(QLabel& label, int frameHeight)
{
    QFontMetrics metrics = label.fontMetrics();
    int labelHeight = getLabelHeight(metrics, frameHeight);

    label.setMinimumHeight(labelHeight);
    label.setMaximumHeight(labelHeight);
}

void RibbonFrame::setLineHeight(QFrame& line, int frameHeight)
{
    int lineHeight = frameHeight * 9 / 10;
    line.setMinimumHeight(lineHeight);
    line.setMaximumHeight(lineHeight);
}

int RibbonFrame::getRibbonHeight(int defaultHeight)
{
    QScreen* primary = QGuiApplication::primaryScreen();
    if(!primary)
        return defaultHeight;

    QSize screenSize = primary->availableSize();

    return screenSize.height() / 15;
}

int RibbonFrame::getLabelHeight(QLabel& label, int frameHeight)
{
    return getLabelHeight(label.fontMetrics(), frameHeight);
}

int RibbonFrame::getLabelHeight(const QFontMetrics& metrics, int frameHeight)
{
    return metrics.height() + (frameHeight / 10);
}

void RibbonFrame::setWidgetSize(QWidget& widget, int w, int h)
{
    widget.setMinimumWidth(w);
    widget.setMaximumWidth(w);
    widget.setMinimumHeight(h);
    widget.setMaximumHeight(h);
}

void RibbonFrame::setButtonSize(QAbstractButton& button, int w, int h)
{
    setWidgetSize(button, w, h);
    int iconSize = getIconSize(w, h);
    button.setIconSize(QSize(iconSize, iconSize));
}

int RibbonFrame::getIconSize(int buttonWidth, int buttonHeight)
{
    return qMin(buttonWidth, buttonHeight) * 4 / 5;
}
