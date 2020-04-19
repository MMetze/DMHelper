#include "publishbuttonribbon.h"
#include "colorpushbutton.h"
#include "ui_publishbuttonribbon.h"
#include <QKeyEvent>
#include <QMessageBox>

PublishButtonRibbon::PublishButtonRibbon(QWidget *parent) :
    RibbonFrame(parent),
    ui(new Ui::PublishButtonRibbon)
{
    ui->setupUi(this);

    connect(ui->btnPublish, SIGNAL(clicked(bool)), this, SIGNAL(clicked(bool)));
    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)));
//    connect(ui->btnPublish, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));

    connect(ui->btnCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SIGNAL(rotateCW()));
    connect(ui->btnCW, SIGNAL(clicked()), this, SLOT(handleRotation()));
    connect(ui->btnCCW, SIGNAL(clicked()), ui->btnColor, SLOT(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SIGNAL(rotateCCW()));
    connect(ui->btnCCW, SIGNAL(clicked()), this, SLOT(handleRotation()));

    connect(ui->btnColor, SIGNAL(rotationChanged(int)), this, SIGNAL(rotationChanged(int)));
    connect(ui->btnColor, SIGNAL(colorChanged(QColor)), this, SIGNAL(colorChanged(QColor)));

    connect(ui->btnPreview, SIGNAL(clicked()), this, SIGNAL(previewClicked()));
    connect(ui->btnPlayersWindow, SIGNAL(clicked(bool)), this, SIGNAL(playersWindowClicked(bool)));

    setDefaults();

    setCheckable(true);
}

PublishButtonRibbon::~PublishButtonRibbon()
{
    delete ui;
}

/*
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
*/

PublishButtonRibbon* PublishButtonRibbon::getPublishRibbon()
{
    return this;
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

void PublishButtonRibbon::setPlayersWindow(bool checked)
{
    ui->btnPlayersWindow->setChecked(checked);
}

void PublishButtonRibbon::showEvent(QShowEvent *event)
{
    RibbonFrame::showEvent(event);

    setStandardButtonSize(*ui->lblPublish, *ui->btnPublish);
    setStandardButtonSize(*ui->lblPreview, *ui->btnPreview);

    setLineHeight(*ui->line_2);

    int labelHeight = getLabelHeight(*ui->lblPublish);
    int buttonSize = height() - labelHeight;
    setButtonSize(*ui->btnPlayersWindow, buttonSize, buttonSize);
    setLineHeight(*ui->line_3, buttonSize);
    setButtonSize(*ui->btnCW, buttonSize, buttonSize);
    setButtonSize(*ui->btnColor, buttonSize, buttonSize);
    setButtonSize(*ui->btnCCW, buttonSize, buttonSize);
    setWidgetSize(*ui->lblPlayersWindow, buttonSize * 4 + 10, labelHeight);
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
