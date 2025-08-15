#ifndef FEARFRAME_H
#define FEARFRAME_H

#include <QFrame>

namespace Ui {
class FearFrame;
}

class Campaign;
class QPushButton;

class FearFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FearFrame(QWidget* parent = nullptr);
    ~FearFrame();

    void setCampaign(Campaign* campaign);

protected slots:
    void buttonClicked();

private:
    void setFear(int fear);

    Ui::FearFrame *ui;

    Campaign* _campaign;
    QMap<int, QPushButton*> _buttonMap;
    QIcon _fearOff;
    QIcon _fearOn;
};

#endif // FEARFRAME_H
