#ifndef QUICKREFFRAME_H
#define QUICKREFFRAME_H

#include <QFrame>

namespace Ui {
class QuickRefFrame;
}

class QuickRefSection;
class QVBoxLayout;

class QuickRefFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QuickRefFrame(QWidget *parent = 0);
    ~QuickRefFrame();

protected:
    void readQuickRef();

protected slots:
    void handleQuickRefChange(int selection);

private:
    Ui::QuickRefFrame *ui;

    QList<QuickRefSection*> _quickRef;
    QVBoxLayout* _quickRefLayout;
};

#endif // QUICKREFFRAME_H
