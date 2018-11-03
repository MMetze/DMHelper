#ifndef DMSCREENTABWIDGET_H
#define DMSCREENTABWIDGET_H

#include <QTabWidget>

namespace Ui {
class DMScreenTabWidget;
}

class DMScreenTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit DMScreenTabWidget(QWidget *parent = 0);
    ~DMScreenTabWidget();

public slots:
    void triggerRoll(const QString& link);

private:
    Ui::DMScreenTabWidget *ui;
};

#endif // DMSCREENTABWIDGET_H
