#ifndef FEARFRAME_H
#define FEARFRAME_H

#include <QFrame>

namespace Ui {
class FearFrame;
}

class Campaign;
class QToolButton;

class FearFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FearFrame(QWidget* parent = nullptr);
    ~FearFrame();

    void setCampaign(Campaign* campaign);

protected slots:
    void buttonClicked();
    void setFear(int fear);

private:

    Ui::FearFrame *ui;

    Campaign* _campaign;
    QMap<int, QToolButton*> _buttonMap;
    QIcon _fearOff;
    QIcon _fearOn;
};

#endif // FEARFRAME_H
