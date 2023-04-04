#include "selectitemdialog.h"
#include "ui_selectitemdialog.h"

SelectItemDialog::SelectItemDialog(const QStringList &items, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectItemDialog)
{
    ui->setupUi(this);
    ui->listWidget->addItems(items);
}

SelectItemDialog::~SelectItemDialog()
{
    delete ui;
}

void SelectItemDialog::setSelectedItem(int index)
{
    if((index < 0) || (index >= ui->listWidget->count()))
        return;

    ui->listWidget->setCurrentRow(index);
}

int SelectItemDialog::getSelectedItem()
{
    return ui->listWidget->currentRow();
}

QString SelectItemDialog::getSelectedLabel()
{
    QListWidgetItem* item = ui->listWidget->currentItem();
    if(item)
        return item->text();
    else
        return QString();

}
