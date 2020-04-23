#include "ribbonframe.h"
#include <QLabel>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>

// DONE - Text ribbon needed - replace both scrolling and text encounter
// DONE - Hide text encounter buttons
// DONE - Publish frame and preview button in ALL ribbons
// DONE - Connect scrolling text
// DONE - Connect all the buttons
// DONE - Player window button under FILE --> NO, moved it to publish frame
// DONE - Combine HELP in FILE, Rename to ???
// DONE - Move Next back to initiative list, fix round counter
// DONE - Scroll name --> Animation
// DONE - Notes don't have an extra unknown node underneath
// Make the TOOLS not modal, but rather hidden when losing focus, with checkable button and escape to close
//      make battle statistics (AND ALL OTHER DIALOGS) dlg wider
// Change MAP and BATTLE layouts to BUILD and RUN
// Add Undo/Redo to text edit

/*
 *
2020.04.20 12:47:44 Warning: [:0, ] QPainter::begin: Paint device returned engine == 0, type: 3
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setPen: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setBrush: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setCompositionMode: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::begin: Paint device returned engine == 0, type: 3
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setPen: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setBrush: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setCompositionMode: Painter not active
2020.04.20 12:47:44 Warning: [:0, ] QPainter::begin: Paint device returned engine == 0, type: 3
2020.04.20 12:47:44 Warning: [:0, ] QPainter::setPen: Painter not active
DMHelperToday at 22:45
Yeah, I need to find the cause for that nonsense
BrianToday at 22:45
Yes, I tried expanding all the way. Window is 2560x1440
DMHelperToday at 22:45
Wow, big. OK
thx
BrianToday at 22:45
The map still does the thing where it shrinks if you make the window smaller but doesn't regrow if you make it bigger, so it eventually vanishes away to nothing
With a lot of
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1762 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1753 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1725 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1724 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1720 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1716 x 1098
2020.04.22 08:40:27 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:27 Debug: [:0, ] [Battle Frame] resized:  1699 x 1098
2020.04.22 08:40:28 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:28 Debug: [:0, ] [Battle Frame] resized:  1698 x 1098
2020.04.22 08:40:28 Debug: [:0, ] "Size " :  QRect(0,-11 20x13) ,  27
2020.04.22 08:40:28 Debug: [:0, ] [Battle Frame] resized:  1697 x 1098
*/

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

    QScreen* primary = QGuiApplication::primaryScreen();
    if(!primary)
        return;

    QSize screenSize = primary->availableSize();

    int ribbonHeight = screenSize.height() / 15;
    setMinimumHeight(ribbonHeight);
    setMaximumHeight(ribbonHeight);
    resize(width(), ribbonHeight);
}

void RibbonFrame::setStandardButtonSize(QLabel& label, QPushButton& button)
{
    QFontMetrics metrics = label.fontMetrics();
    int labelHeight = getLabelHeight(metrics);
    int iconDim = height() - labelHeight;
    int newWidth = qMax(metrics.horizontalAdvance(label.text()), iconDim);

    setWidgetSize(label, newWidth, labelHeight);
    setButtonSize(button, newWidth, iconDim);
    //setWidgetSize(button, newWidth, iconDim);
    //button.setIconSize(QSize(iconDim * 4 / 5, iconDim * 4 / 5));
}

void RibbonFrame::setLineHeight(QFrame& line)
{
    setLineHeight(line, height());
}

void RibbonFrame::setLineHeight(QFrame& line, int fullHeight)
{
    int lineHeight = fullHeight * 9 / 10;
    line.setMinimumHeight(lineHeight);
    line.setMaximumHeight(lineHeight);
}

int RibbonFrame::getLabelHeight(QLabel& label) const
{
    return getLabelHeight(label.fontMetrics());
}

int RibbonFrame::getLabelHeight(const QFontMetrics& metrics) const
{
    return metrics.height() + (height() / 10);
}

void RibbonFrame::setWidgetSize(QWidget& widget, int w, int h) const
{
    widget.setMinimumWidth(w);
    widget.setMaximumWidth(w);
    widget.setMinimumHeight(h);
    widget.setMaximumHeight(h);
}

void RibbonFrame::setButtonSize(QPushButton& button, int w, int h) const
{
    setWidgetSize(button, w, h);
    int iconSize = getIconSize(w, h);
    button.setIconSize(QSize(iconSize, iconSize));
}

int RibbonFrame::getIconSize(int buttonWidth, int buttonHeight) const
{
    return qMin(buttonWidth, buttonHeight) * 4 / 5;
}
