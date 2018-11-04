#include "mapmarkerdialog.h"
#include "ui_mapmarkerdialog.h"

// TODO: add encounter links

MapMarkerDialog::MapMarkerDialog(QString title, QString description, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapMarkerDialog)
{
    ui->setupUi(this);

    ui->edtTitle->setText(title);
    ui->txtDescription->setPlainText(description);
}

MapMarkerDialog::~MapMarkerDialog()
{
    delete ui;
}

QString MapMarkerDialog::getTitle()
{
    return ui->edtTitle->text();
}

QString MapMarkerDialog::getDescription()
{
    return ui->txtDescription->toPlainText();
}
