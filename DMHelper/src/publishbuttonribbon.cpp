#include "publishbuttonribbon.h"
#include "colorpushbutton.h"
#include "ui_publishbuttonribbon.h"
#include <QKeyEvent>
#include <QMessageBox>

PublishButtonRibbon::PublishButtonRibbon(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PublishButtonRibbon)
{
    ui->setupUi(this);

    connect(ui->btnPublish, SIGNAL(clicked(bool)), this, SIGNAL(clicked(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));

    connect(ui->btnCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SIGNAL(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SLOT(handleRotation()));
    connect(ui->btnCCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SIGNAL(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SLOT(handleRotation()));

    connect(ui->btnColor, SIGNAL(rotationChanged(int)), this, SIGNAL(rotationChanged(int)));
    connect(ui->btnColor, SIGNAL(colorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));

    setDefaults();

    setCheckable(true);
}

PublishButtonRibbon::~PublishButtonRibbon()
{
    delete ui;
}

bool PublishButtonRibbon::isChecked()
{
    return ui->btnPublish->isChecked();
}

bool PublishButtonRibbon::isCheckable()
{
    return ui->btnPublish->isCheckable();
}

QColor PublishButtonRibbon::getColor() const
{
    return ui->btnColor->getColor();
}

int PublishButtonRibbon::getRotation()
{
    return ui->btnColor->getRotation();
}

void PublishButtonRibbon::setChecked(bool checked)
{
    ui->btnPublish->setChecked(checked);
}

void PublishButtonRibbon::setCheckable(bool checkable)
{
    ui->btnPublish->setCheckable(checkable);
}

void PublishButtonRibbon::setRotation(int rotation)
{
    ui->btnColor->setRotation(rotation);
}

void PublishButtonRibbon::setColor(QColor color)
{
    ui->btnColor->setColor(color);
}

void PublishButtonRibbon::cancelPublish()
{
    setChecked(false);
}

void PublishButtonRibbon::handleToggle(bool checked)
{
    if(checked)
    {
        //ui->btnPublish->setStyleSheet(QString("QPushButton {color: red; font-weight: bold; }"));
        //ui->btnPublish->setText(QString("Publishing!"));
        ui->btnPublish->setIcon(QIcon(QPixmap(":/img/data/icon_publishon.png")));
    }
    else
    {
        //ui->btnPublish->setStyleSheet(QString("QPushButton {color: black; font-weight: bold; }"));
        //ui->btnPublish->setText(QString("Publish"));
        ui->btnPublish->setIcon(QIcon(QPixmap(":/img/data/icon_publish.png")));
    }
}

void PublishButtonRibbon::handleRotation()
{
    emit rotationChanged(ui->btnColor->getRotation());
}

void PublishButtonRibbon::setDefaults()
{
    ui->btnPublish->setAutoDefault(false);
    ui->btnCW->setAutoDefault(false);
    ui->btnCCW->setAutoDefault(false);
    ui->btnColor->setAutoDefault(false);

    ui->btnPublish->setDefault(false);
    ui->btnCW->setDefault(false);
    ui->btnCCW->setDefault(false);
    ui->btnColor->setDefault(false);
}
