#ifndef QUICKREFDATAWIDGET_H
#define QUICKREFDATAWIDGET_H

#include <QWidget>
#include "quickref.h"

namespace Ui {
class QuickRefDataWidget;
}

class QuickRefDataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QuickRefDataWidget(const QuickRefData& data, QWidget *parent = nullptr);
    ~QuickRefDataWidget();

    //virtual QSize sizeHint() const;

protected:

    // From QWidget
    void leaveEvent(QEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

    // Local
    void setIcon(const QString& icon);

private:
    Ui::QuickRefDataWidget *ui;

    void showQuickRefDetails();

    // Data
    const QuickRefData& _data;
    Qt::MouseButton _mouseDown;
};

#endif // QUICKREFDATAWIDGET_H
