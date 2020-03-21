#include "ribbontabmap.h"
#include "ui_ribbontabmap.h"

RibbonTabMap::RibbonTabMap(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RibbonTabMap)
{
    ui->setupUi(this);
}

RibbonTabMap::~RibbonTabMap()
{
    delete ui;
}

void RibbonTabMap::setDistance(const QString& distance)
{
    ui->edtDistance->setText(distance);
}
