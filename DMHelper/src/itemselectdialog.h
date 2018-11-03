#ifndef ITEMSELECTDIALOG_H
#define ITEMSELECTDIALOG_H

#include <QDialog>
#include <QVariant>

namespace Ui {
class ItemSelectDialog;
}

class ItemSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ItemSelectDialog(QWidget *parent = 0);
    ~ItemSelectDialog();

    void setLabel(const QString& label);
    int getItemCount() const;

    void addItem(const QString &text, const QVariant &userData = QVariant());
    void addItem(const QString &text, const QVariant &userData1, const QVariant &userData2);
    void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
    void addSeparator(int index);

    int getSelectedIndex() const;
    QVariant getSelectedData() const;
    QVariant getSelectedData2() const;
    QString getSelectedString() const;

private:
    Ui::ItemSelectDialog *ui;
};

#endif // ITEMSELECTDIALOG_H
