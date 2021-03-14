#include "selectstringdialog.h"
#include "ui_selectstringdialog.h"

SelectStringDialog::SelectStringDialog(QStringList entries, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectStringDialog),
    _entries()
{
    ui->setupUi(this);

    connect(ui->btnSelectAll, &QAbstractButton::clicked, this, &SelectStringDialog::selectAll);
    connect(ui->btnSelectNone, &QAbstractButton::clicked, this, &SelectStringDialog::selectNone);

    addEntries(entries);
}

SelectStringDialog::~SelectStringDialog()
{
    delete ui;
}

QStringList SelectStringDialog::getSelection()
{
    QStringList result;

    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        if((ui->listWidget->item(i)) && (ui->listWidget->item(i)->checkState() == Qt::Checked))
            result.append(ui->listWidget->item(i)->text());
    }

    return result;
}

void SelectStringDialog::selectAll()
{
    changeAllSelections(true);
}

void SelectStringDialog::selectNone()
{
    changeAllSelections(false);
}

void SelectStringDialog::addEntries(QStringList entries)
{
    for(QString entry : entries)
        addEntry(entry);
}

void SelectStringDialog::addEntry(const QString& entry, bool checked, const QIcon& icon)
{
    if(_entries.contains(entry))
        return;

    QListWidgetItem* newItem = new QListWidgetItem(icon, entry);
    newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
    newItem->setCheckState(checked ? Qt::Checked : Qt::Unchecked);

    ui->listWidget->addItem(newItem);
    _entries.append(entry);
}

void SelectStringDialog::changeAllSelections(bool checked)
{
    Qt::CheckState checkState = checked ? Qt::Checked : Qt::Unchecked;

    for(int i = 0; i < ui->listWidget->count(); ++i)
    {
        if(ui->listWidget->item(i))
            ui->listWidget->item(i)->setCheckState(checkState);
    }
}
