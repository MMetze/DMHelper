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
    explicit QuickRefFrame(const QString& quickRefFile, QWidget *parent = nullptr);
    ~QuickRefFrame();

public slots:
    void readQuickRef(const QString& quickRefFile);

protected slots:
    void handleQuickRefChange(int selection);

private:
    Ui::QuickRefFrame *ui;

    QList<QuickRefSection*> _quickRef;
    QVBoxLayout* _quickRefLayout;
};

#endif // QUICKREFFRAME_H
