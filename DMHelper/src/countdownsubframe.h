#ifndef COUNTDOWNSUBFRAME_H
#define COUNTDOWNSUBFRAME_H

#include <QFrame>

namespace Ui {
class CountdownSubFrame;
}

class CountdownSubFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CountdownSubFrame(QWidget *parent = nullptr);
    ~CountdownSubFrame();

    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;

signals:
    void hoursEdited();
    void minutesEdited();
    void secondsEdited();

public slots:
    void setReadOnly(bool readOnly);
    void setHours(int hours);
    void setMinutes(int minutes);
    void setSeconds(int seconds);
    void setTime(int hours, int minutes, int seconds);

private:
    Ui::CountdownSubFrame *ui;
};

#endif // COUNTDOWNSUBFRAME_H
