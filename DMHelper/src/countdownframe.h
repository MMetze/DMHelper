#ifndef COUNTDOWNFRAME_H
#define COUNTDOWNFRAME_H

#include <QFrame>

namespace Ui {
class CountdownFrame;
}

class CountdownFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CountdownFrame(QWidget *parent = nullptr);
    ~CountdownFrame();

signals:
    void hoursChanged(int hours);
    void minutesChanged(int minutes);
    void secondsChanged(int seconds);

protected:
    // From QObject
    virtual void timerEvent(QTimerEvent *event);

private slots:
    void targetChanged();
    void startStopClicked();
    void resetClicked();
    void publishClicked(bool checked);

private:
    Ui::CountdownFrame *ui;

    int _hoursTarget;
    int _minutesTarget;
    int _secondsTarget;
    int _hoursCurrent;
    int _minutesCurrent;
    int _secondsCurrent;
    int _timerId;
    QDialog* _publishDlg;
};

#endif // COUNTDOWNFRAME_H
