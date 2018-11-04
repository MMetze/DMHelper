#include "timeanddateframe.h"
#include "ui_timeanddateframe.h"

// Todo: handle changing days by rotating the clock several times.

TimeAndDateFrame::TimeAndDateFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::TimeAndDateFrame),
    _time(0,0),
    _date(1,1,0)
{
    ui->setupUi(this);
    ui->frame->setSmallHandActive(false);
    ui->frame->setSmallHandAngle(0.f);
    ui->frame->setLargeHandAngle(0.f);

    connect(ui->btnNextDay,SIGNAL(clicked(bool)),this,SLOT(nextDay()));
    connect(ui->dateEdit,SIGNAL(textEdited(QString)),this,SLOT(dateEdited(QString)));
    connect(ui->timeEdit,SIGNAL(timeChanged(QTime)),this,SLOT(setTimeTarget(QTime)));
    connect(ui->frame,SIGNAL(handsChanged()),this,SLOT(handsChanged()));
}

TimeAndDateFrame::~TimeAndDateFrame()
{
    delete ui;
}

BasicDate TimeAndDateFrame::getDate() const
{
    return _date;
}

QTime TimeAndDateFrame::getTime() const
{
    return _time;
}

int TimeAndDateFrame::getTimeMSec() const
{
    return _time.msecsSinceStartOfDay();
}

void TimeAndDateFrame::setDate(const BasicDate &date)
{
    if(date != _date)
    {
        _date = date;
        ui->dateEdit->setText(date.toStringDDMMYYYY());
        ui->frame->setSmallHandAngle(360.f * (float)(date.dayOfYear() - 1) / (float)(date.daysInYear()));
        emit dateChanged(_date);
    }
}

void TimeAndDateFrame::setTime(const QTime& time)
{
    if(_time != time)
    {
        _time = time;
        ui->timeEdit->setTime(time);
        ui->frame->setLargeHandAngle((float)(time.msecsSinceStartOfDay()) * 360.f / 86400000.f);
        emit timeChanged(_time);
    }
}

void TimeAndDateFrame::setTimeMSec(int msecs)
{
    setTime(QTime::fromMSecsSinceStartOfDay(msecs));
}

QSize TimeAndDateFrame::sizeHint() const
{
    return QSize(322, 391);
}

void TimeAndDateFrame::nextDay()
{
    BasicDate today = getDate();
    today.addDay();
    setDate(today);
    setTimeTarget(QTime(7,0));
}

void TimeAndDateFrame::dateEdited(const QString &text)
{
    BasicDate newDate(text);
    if(newDate.isValid())
    {
        setDate(newDate);
    }
}

void TimeAndDateFrame::handsChanged()
{
    setTimeMSec(ui->frame->getLargeHandAngle() * 86400000.f / 360.f);
}

void TimeAndDateFrame::setTimeTarget(const QTime &time)
{
    if(_time != time)
    {
        _time = time;
        ui->timeEdit->setTime(time);
        ui->frame->setLargeHandAngleTarget((float)(time.msecsSinceStartOfDay()) * 360.f / 86400000.f);
        emit timeChanged(_time);
    }
}
