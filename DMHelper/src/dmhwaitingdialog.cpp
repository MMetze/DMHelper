#include "dmhwaitingdialog.h"
#include "ui_dmhwaitingdialog.h"
#include "dmconstants.h"
#include <QPainter>
#include <QDebug>

DMHWaitingDialog::DMHWaitingDialog(const QString& statusString, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DMHWaitingDialog),
    _dieImage(),
    _backgroundImage(),
    _elapsed(),
    _timerId(0),
    _rotation(0.0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint);

    _backgroundImage.load(":/img/data/dmhelper_waiting_image");
    _dieImage.load(":/img/data/dmhelper_waiting_die");
    ui->lblStatus->setText(statusString);

    ui->label->installEventFilter(this);
}

DMHWaitingDialog::~DMHWaitingDialog()
{
    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }

    delete ui;
}

void DMHWaitingDialog::setStatus(const QString& statusString)
{
    ui->lblStatus->setText(statusString);
}

void DMHWaitingDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    if(_timerId)
    {
        killTimer(_timerId);
        _timerId = 0;
    }
}

void DMHWaitingDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if(parentWidget())
        move((parentWidget()->width() / 2) - (width() / 2), (parentWidget()->height() / 2) - (height() / 2));

    if(!_timerId)
    {
        _elapsed.start();
        _timerId = startTimer(DMHelper::ANIMATION_TIMER_DURATION, Qt::PreciseTimer);
    }
}

void DMHWaitingDialog::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    qreal elapsedtime = _elapsed.restart();
    _rotation += 180.0 * elapsedtime / 1000.0;
    ui->label->update();
}

bool DMHWaitingDialog::eventFilter(QObject *watched, QEvent *event)
{
    if((watched == ui->label) && (event->type() == QEvent::Paint))
    {
        QImage rotatedDie = _dieImage.transformed(QTransform().rotate(_rotation));
        QPainter paint(ui->label);
        paint.drawImage(0, 0, _backgroundImage);
        //paint.rotate(_rotation);
        paint.drawImage(156 - (rotatedDie.width() / 2), 143 - (rotatedDie.height() / 2), rotatedDie);
        return true;
    }

    return QDialog::eventFilter(watched, event);
}