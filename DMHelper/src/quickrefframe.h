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
    explicit QuickRefFrame(QWidget *parent = nullptr);
    ~QuickRefFrame();

public slots:
    void refreshQuickRef();
    void setQuickRefSection(const QString& sectionName);

protected slots:
    void handleQuickRefChange(int selection);

private:
    Ui::QuickRefFrame *ui;

    QVBoxLayout* _quickRefLayout;
};

#endif // QUICKREFFRAME_H
