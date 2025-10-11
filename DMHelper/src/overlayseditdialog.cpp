#include "overlayseditdialog.h"
#include "ui_overlayseditdialog.h"

OverlaysEditDialog::OverlaysEditDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::OverlaysEditDialog)
{
    ui->setupUi(this);
}

OverlaysEditDialog::~OverlaysEditDialog()
{
    delete ui;
}
