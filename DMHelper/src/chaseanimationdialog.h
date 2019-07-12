#ifndef CHASEANIMATIONDIALOG_H
#define CHASEANIMATIONDIALOG_H

#include <QDialog>

class QGraphicsView;
class QResizeEvent;

namespace Ui {
class ChaseAnimationDialog;
}

class ChaseAnimationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChaseAnimationDialog(QWidget *parent = nullptr);
    ~ChaseAnimationDialog();

    QGraphicsView* getGraphicsView() const;

protected:
    virtual void resizeEvent(QResizeEvent* event);

private:
    Ui::ChaseAnimationDialog *ui;
};

#endif // CHASEANIMATIONDIALOG_H
