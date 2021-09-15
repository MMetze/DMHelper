#include "mapblankdialog.h"
#include "ui_mapblankdialog.h"
#include <QIntValidator>

MapBlankDialog::MapBlankDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MapBlankDialog)
{
    ui->setupUi(this);
    ui->btnColor->setRotationVisible(false);
    ui->btnColor->setColor(Qt::white);

    ui->edtWidth->setValidator(new QIntValidator(1,9999,this));
    ui->edtHeight->setValidator(new QIntValidator(1,9999,this));
}

MapBlankDialog::~MapBlankDialog()
{
    delete ui;
}

QColor MapBlankDialog::getMapColor() const
{
    return ui->btnColor->getColor();
}

int MapBlankDialog::getMapWidth() const
{
    return ui->edtWidth->text().toInt();
}

int MapBlankDialog::getMapHeight() const
{
    return ui->edtHeight->text().toInt();
}

QSize MapBlankDialog::getMapSize() const
{
    return QSize(getMapWidth(), getMapHeight());
}
