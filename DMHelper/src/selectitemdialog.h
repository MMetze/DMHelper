#ifndef SELECTITEMDIALOG_H
#define SELECTITEMDIALOG_H

#include <QDialog>

namespace Ui {
class SelectItemDialog;
}

class SelectItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectItemDialog(const QStringList &items, QWidget *parent = nullptr);
    ~SelectItemDialog();

    void setSelectedItem(int index);
    int getSelectedItem();
    QString getSelectedLabel();

private:
    Ui::SelectItemDialog *ui;
};

#endif // SELECTITEMDIALOG_H
