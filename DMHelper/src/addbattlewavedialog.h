#ifndef ADDBATTLEWAVEDIALOG_H
#define ADDBATTLEWAVEDIALOG_H

#include <QDialog>

namespace Ui {
class AddBattleWaveDialog;
}

class EncounterBattle;

class AddBattleWaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBattleWaveDialog(EncounterBattle* battle, QWidget *parent = nullptr);
    ~AddBattleWaveDialog();

    int selectedWave() const;

public slots:
    void selectWave(int wave);

private:
    Ui::AddBattleWaveDialog *ui;

    EncounterBattle* _battle;
};

#endif // ADDBATTLEWAVEDIALOG_H
