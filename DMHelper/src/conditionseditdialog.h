#ifndef CONDITIONSEDITDIALOG_H
#define CONDITIONSEDITDIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class ConditionsEditDialog;
}

class ConditionsEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConditionsEditDialog(QWidget *parent = nullptr);
    ~ConditionsEditDialog();

    void setConditions(int conditions);
    int getConditions() const;

protected slots:
    void setExhausted(bool exhausted);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    void setButtonSize(QLabel& label, QPushButton& button, int frameHeight, int buttonWidth);

    Ui::ConditionsEditDialog *ui;
};

#endif // CONDITIONSEDITDIALOG_H
