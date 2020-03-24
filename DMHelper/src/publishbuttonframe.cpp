#include "publishbuttonframe.h"
#include "colorpushbutton.h"
#include "ui_publishbuttonframe.h"
#include <QKeyEvent>
#include <QMessageBox>

PublishButtonFrame::PublishButtonFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PublishButtonFrame)
{
    ui->setupUi(this);

    connect(ui->btnPublish, SIGNAL(clicked(bool)), this, SIGNAL(clicked(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));

    connect(ui->btnCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SIGNAL(rotateCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SIGNAL(rotateCCW()));

    connect(ui->btnColor, SIGNAL(colorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));

    setDefaults();
}

PublishButtonFrame::~PublishButtonFrame()
{
    delete ui;
}

bool PublishButtonFrame::isChecked()
{
    return ui->btnPublish->isChecked();
}

bool PublishButtonFrame::isCheckable()
{
    return ui->btnPublish->isCheckable();
}

void PublishButtonFrame::setCheckable(bool checkable)
{
    ui->btnPublish->setCheckable(checkable);
}

QColor PublishButtonFrame::getColor() const
{
    return ui->btnColor->getColor();
}

int PublishButtonFrame::getRotation()
{
    return ui->btnColor->getRotation();
}

void PublishButtonFrame::setChecked(bool checked)
{
    ui->btnPublish->setChecked(checked);
}

void PublishButtonFrame::setRotation(int rotation)
{
    ui->btnColor->setRotation(rotation);
}

void PublishButtonFrame::setColor(QColor color)
{
    ui->btnColor->setColor(color);
}

void PublishButtonFrame::cancelPublish()
{
    setChecked(false);
}

void PublishButtonFrame::handleToggle(bool checked)
{
    if(checked)
    {
        ui->btnPublish->setStyleSheet(QString("QPushButton {color: red; font-weight: bold; }"));
        ui->btnPublish->setText(QString("Publishing!"));
    }
    else
    {
        ui->btnPublish->setStyleSheet(QString("QPushButton {color: black; font-weight: bold; }"));
        ui->btnPublish->setText(QString("Publish"));
    }
}

void PublishButtonFrame::setDefaults()
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
