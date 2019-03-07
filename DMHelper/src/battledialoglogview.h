#ifndef BATTLEDIALOGLOGVIEW_H
#define BATTLEDIALOGLOGVIEW_H

#include <QDialog>

class BattleDialogModel;
class BattleDialogLogger;

namespace Ui {
class BattleDialogLogView;
}

class BattleDialogLogView : public QDialog
{
    Q_OBJECT

public:
    explicit BattleDialogLogView(const BattleDialogModel& model, const BattleDialogLogger& logger, QWidget *parent = nullptr);
    ~BattleDialogLogView();

private:
    Ui::BattleDialogLogView *ui;
};

#endif // BATTLEDIALOGLOGVIEW_H
