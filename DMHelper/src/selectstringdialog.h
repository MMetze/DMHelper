#ifndef SELECTSTRINGDIALOG_H
#define SELECTSTRINGDIALOG_H

#include <QDialog>
#include <QIcon>

namespace Ui {
class SelectStringDialog;
}

class SelectStringDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectStringDialog(QStringList entries = QStringList(), QWidget *parent = nullptr);
    ~SelectStringDialog();

    QStringList getSelection();

public slots:
    void selectAll();
    void selectNone();
    void addEntries(QStringList entries);
    void addEntry(const QString& entry, bool checked = false, const QIcon& icon = QIcon());

private:
    void changeAllSelections(bool checked);

    Ui::SelectStringDialog *ui;
    QStringList _entries;
};

#endif // SELECTSTRINGDIALOG_H
