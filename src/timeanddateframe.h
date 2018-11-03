#ifndef TIMEANDDATEFRAME_H
#define TIMEANDDATEFRAME_H

#include <QFrame>
#include "basicdate.h"
#include <QTime>

namespace Ui {
class TimeAndDateFrame;
}

class TimeAndDateFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TimeAndDateFrame(QWidget *parent = 0);
    ~TimeAndDateFrame();

    BasicDate getDate() const;
    QTime getTime() const;
    int getTimeMSec() const;

    virtual QSize sizeHint() const;

public slots:
    void setDate(const BasicDate &date);
    void setTime(const QTime &time);
    void setTimeMSec(int msecs);
    void nextDay();

signals:
    void dateChanged(const BasicDate& date);
    void timeChanged(const QTime& time);

private slots:
    void dateEdited(const QString &text);
    void handsChanged();
    void setTimeTarget(const QTime &time);

private:
    Ui::TimeAndDateFrame *ui;

    QTime _time;
    BasicDate _date;
};

#endif // TIMEANDDATEFRAME_H
