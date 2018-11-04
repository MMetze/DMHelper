#include "itemselectdialog.h"
#include "ui_itemselectdialog.h"

ItemSelectDialog::ItemSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemSelectDialog)
{
    ui->setupUi(this);
}

ItemSelectDialog::~ItemSelectDialog()
{
    delete ui;
}

void ItemSelectDialog::setLabel(const QString& label)
{
    ui->lblSelect->setText(label);
}

int ItemSelectDialog::getItemCount() const
{
    return ui->cmbSelect->count();
}

void ItemSelectDialog::addItem(const QString &text, const QVariant &userData)
{
    ui->cmbSelect->addItem(text, userData);
}

void ItemSelectDialog::addItem(const QString &text, const QVariant &userData1, const QVariant &userData2)
{
    ui->cmbSelect->addItem(text, userData1);
    ui->cmbSelect->setItemData(ui->cmbSelect->count() - 1, userData2, Qt::UserRole + 1);
}

void ItemSelectDialog::addItem(const QIcon &icon, const QString &text, const QVariant &userData)
{
    ui->cmbSelect->addItem(icon, text, userData);
}

void ItemSelectDialog::addSeparator(int index)
{
    ui->cmbSelect->insertSeparator(index);
}

int ItemSelectDialog::getSelectedIndex() const
{
    return ui->cmbSelect->currentIndex();
}

QVariant ItemSelectDialog::getSelectedData() const
{
    return ui->cmbSelect->currentData();
}

QVariant ItemSelectDialog::getSelectedData2() const
{
    return ui->cmbSelect->currentData(Qt::UserRole + 1);
}

QString ItemSelectDialog::getSelectedString() const
{
    return ui->cmbSelect->currentText();
}
