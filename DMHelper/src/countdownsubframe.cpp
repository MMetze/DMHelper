#include "countdownsubframe.h"
#include "ui_countdownsubframe.h"
#include <QIntValidator>

CountdownSubFrame::CountdownSubFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CountdownSubFrame)
{
    ui->setupUi(this);

    ui->edtHours->setValidator(new QIntValidator(0, 99));
    ui->edtMinutes->setValidator(new QIntValidator(0, 99));
    ui->edtSeconds->setValidator(new QIntValidator(0, 99));

    connect(ui->edtHours, SIGNAL(editingFinished()), this, SIGNAL(hoursEdited()));
    connect(ui->edtMinutes, SIGNAL(editingFinished()), this, SIGNAL(minutesEdited()));
    connect(ui->edtSeconds, SIGNAL(editingFinished()), this, SIGNAL(secondsEdited()));

}

CountdownSubFrame::~CountdownSubFrame()
{
    delete ui;
}

int CountdownSubFrame::getHours() const
{
    return ui->edtHours->text().toInt();
}

int CountdownSubFrame::getMinutes() const
{
    return ui->edtMinutes->text().toInt();
}

int CountdownSubFrame::getSeconds() const
{
    return ui->edtSeconds->text().toInt();
}

void CountdownSubFrame::setReadOnly(bool readOnly)
{
    ui->edtHours->setReadOnly(readOnly);
    ui->edtMinutes->setReadOnly(readOnly);
    ui->edtSeconds->setReadOnly(readOnly);
}

void CountdownSubFrame::setHours(int hours)
{
    if((hours < 0) || (hours > 99))
        return;

    ui->edtHours->setText(QString::number(hours));
}

void CountdownSubFrame::setMinutes(int minutes)
{
    if((minutes < 0) || (minutes > 99))
        return;

    ui->edtMinutes->setText(QString::number(minutes));
}

void CountdownSubFrame::setSeconds(int seconds)
{
    if((seconds < 0) || (seconds > 99))
        return;

    ui->edtSeconds->setText(QString::number(seconds));
}

void CountdownSubFrame::setTime(int hours, int minutes, int seconds)
{
    setHours(hours);
    setMinutes(minutes);
    setSeconds(seconds);
}
