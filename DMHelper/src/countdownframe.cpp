#include "countdownframe.h"
#include "ui_countdownframe.h"
#include <QDialog>

CountdownFrame::CountdownFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::CountdownFrame),
    _hoursTarget(0),
    _minutesTarget(5),
    _secondsTarget(0),
    _hoursCurrent(0),
    _minutesCurrent(0),
    _secondsCurrent(0),
    _timerId(-1),
    _publishDlg(nullptr)
{
    ui->setupUi(this);

    resetClicked();

    connect(ui->btnStartStop, SIGNAL(clicked(bool)), this, SLOT(startStopClicked()));
    connect(ui->btnReset, SIGNAL(clicked(bool)), this, SLOT(resetClicked()));
    connect(ui->btnPublish, SIGNAL(clicked(bool)), this, SLOT(publishClicked(bool)));

    connect(ui->frame, SIGNAL(hoursEdited()), this, SLOT(targetChanged()));
    connect(ui->frame, SIGNAL(minutesEdited()), this, SLOT(targetChanged()));
    connect(ui->frame, SIGNAL(secondsEdited()), this, SLOT(targetChanged()));
    connect(this, SIGNAL(hoursChanged(int)), ui->frame, SLOT(setHours(int)));
    connect(this, SIGNAL(minutesChanged(int)), ui->frame, SLOT(setMinutes(int)));
    connect(this, SIGNAL(secondsChanged(int)), ui->frame, SLOT(setSeconds(int)));
}

CountdownFrame::~CountdownFrame()
{
    delete _publishDlg;
    delete ui;
}

void CountdownFrame::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    --_secondsCurrent;
    if(_secondsCurrent < 0)
    {
        _secondsCurrent = 0;
        --_minutesCurrent;
        if(_minutesCurrent < 0)
        {
            _minutesCurrent = 0;
            --_hoursCurrent;
            if(_hoursCurrent < 0)
            {
                _hoursCurrent = 0;
                ui->btnStartStop->click();
            }
            else
            {
                _minutesCurrent = 59;
                _secondsCurrent = 59;
            }

            //ui->edtHours->setText(QString::number(_hoursCurrent));
            //ui->frame->setHours(_hoursCurrent);
            emit hoursChanged(_hoursCurrent);
        }
        else
        {
            _secondsCurrent = 59;
        }

        //ui->edtMinutes->setText(QString::number(_minutesCurrent));
        //ui->frame->setMinutes(_minutesCurrent);
        emit minutesChanged(_minutesCurrent);
    }

    //ui->edtSeconds->setText(QString::number(_secondsCurrent));
    //ui->frame->setSeconds(_secondsCurrent);
    emit secondsChanged(_secondsCurrent);
}

void CountdownFrame::targetChanged()
{
    _hoursTarget = ui->frame->getHours();
    _minutesTarget = ui->frame->getMinutes();
    _secondsTarget = ui->frame->getSeconds();

    resetClicked();
}

void CountdownFrame::startStopClicked()
{
    if(ui->btnStartStop->isChecked())
    {
        _timerId = startTimer(1000);
        ui->btnStartStop->setText(QString("Stop"));
    }
    else
    {
        killTimer(_timerId);
        _timerId = -1;
        ui->btnStartStop->setText(QString("Start"));
    }
}

void CountdownFrame::resetClicked()
{
    _hoursCurrent = _hoursTarget;
    _minutesCurrent = _minutesTarget;
    _secondsCurrent = _secondsTarget;

    //ui->edtHours->setText(QString::number(_hoursCurrent));
    //ui->edtMinutes->setText(QString::number(_minutesCurrent));
    //ui->edtSeconds->setText(QString::number(_secondsCurrent));
    emit hoursChanged(_hoursCurrent);
    emit minutesChanged(_minutesCurrent);
    emit secondsChanged(_secondsCurrent);
}

void CountdownFrame::publishClicked(bool checked)
{
    if(checked)
    {
        if(_publishDlg)
            return;

        _publishDlg = new QDialog(this);
        QVBoxLayout* vLayout = new QVBoxLayout();

        CountdownSubFrame* subFrame = new CountdownSubFrame(_publishDlg);
        subFrame->setReadOnly(true);
        connect(this, SIGNAL(hoursChanged(int)), subFrame, SLOT(setHours(int)));
        connect(this, SIGNAL(minutesChanged(int)), subFrame, SLOT(setMinutes(int)));
        connect(this, SIGNAL(secondsChanged(int)), subFrame, SLOT(setSeconds(int)));
        subFrame->setTime(_hoursCurrent, _minutesCurrent, _secondsCurrent);

        vLayout->addWidget(subFrame);
        _publishDlg->setLayout(vLayout);
        _publishDlg->setWindowFlags(Qt::Window);
        _publishDlg->show();
    }
    else
    {
        delete _publishDlg;
        _publishDlg = nullptr;
    }
}
