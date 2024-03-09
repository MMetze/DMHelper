#ifndef BATTLEDIALOGEFFECTSETTINGSBASE_H
#define BATTLEDIALOGEFFECTSETTINGSBASE_H

#include <QDialog>

class BattleDialogModelEffect;

class BattleDialogEffectSettingsBase : public QDialog
{
    Q_OBJECT
public:
    explicit BattleDialogEffectSettingsBase(QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsBase();

    // Base clase implementations
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) = 0;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) = 0;

};

#endif // BATTLEDIALOGEFFECTSETTINGSBASE_H
