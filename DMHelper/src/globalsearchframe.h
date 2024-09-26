#ifndef GLOBALSEARCHFRAME_H
#define GLOBALSEARCHFRAME_H

#include <QFrame>

namespace Ui {
class GlobalSearchFrame;
}

class Campaign;

class GlobalSearchFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GlobalSearchFrame(QWidget *parent = nullptr);
    ~GlobalSearchFrame();

    void setCampaign(Campaign* campaign);

private:
    Ui::GlobalSearchFrame *ui;

    Campaign* _campaign;
};

#endif // GLOBALSEARCHFRAME_H
