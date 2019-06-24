#include "timeanddateframe.h"
#include "ui_timeanddateframe.h"
#include "basicdateserver.h"
#include <QLineEdit>
#include <QComboBox>
#include <QIntValidator>

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

    ui->edtDay->setValidator(new QIntValidator());
    ui->edtYear->setValidator(new QIntValidator());

    connect(ui->btnNextDay,SIGNAL(clicked(bool)),this,SLOT(nextDay()));
    connect(ui->cmbCalendar,SIGNAL(activated(int)),this,SLOT(calendarChanged()));
    connect(ui->edtDay,SIGNAL(textEdited(QString)),this,SLOT(dateEdited()));
    connect(ui->cmbMonth,SIGNAL(activated(int)),this,SLOT(dateEdited()));
    connect(ui->edtYear,SIGNAL(textEdited(QString)),this,SLOT(dateEdited()));
    connect(ui->timeEdit,SIGNAL(timeChanged(QTime)),this,SLOT(setTimeTarget(QTime)));
    connect(ui->frame,SIGNAL(handsChanged()),this,SLOT(handsChanged()));

    if(BasicDateServer::Instance())
    {
        ui->cmbCalendar->addItems(BasicDateServer::Instance()->getCalendarNames());
        ui->cmbCalendar->setCurrentText(BasicDateServer::Instance()->getActiveCalendarName());

        ui->cmbMonth->addItems(BasicDateServer::Instance()->getMonthNamesWithAlternatives());
    }
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
        setDateVisualization();
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
    return QSize(445, 630);
}

void TimeAndDateFrame::nextDay()
{
    BasicDate today = getDate();
    today.addDay();
    setDate(today);
    setTimeTarget(QTime(7,0));
}

void TimeAndDateFrame::dateEdited()
{
    BasicDate newDate(ui->edtDay->text().toInt(), ui->cmbMonth->currentIndex() + 1, ui->edtYear->text().toInt());
    if(newDate.isValid())
    {
        setDate(newDate);
    }
}

void TimeAndDateFrame::handsChanged()
{
    setTimeMSec(ui->frame->getLargeHandAngle() * 86400000.f / 360.f);
}

void TimeAndDateFrame::calendarChanged()
{
    if(!BasicDateServer::Instance())
        return;

    BasicDateServer::Instance()->setActiveCalendar(ui->cmbCalendar->currentText());

    ui->cmbMonth->clear();
    ui->cmbMonth->addItems(BasicDateServer::Instance()->getMonthNamesWithAlternatives());

    setDateVisualization();
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

void TimeAndDateFrame::setDateVisualization()
{
    ui->edtDay->setText(QString::number(_date.day()));
    if((_date.month() > 0) && (_date.month() <= ui->cmbMonth->count()))
        ui->cmbMonth->setCurrentIndex(_date.month() - 1);
    ui->edtYear->setText(QString::number(_date.year()));

    ui->frame->setSmallHandAngle(360.f * (float)(_date.dayOfYear() - 1) / (float)(_date.daysInYear()));
}
