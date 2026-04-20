#ifndef BATTLEDIALOGEFFECTSETTINGSSPARKS_H
#define BATTLEDIALOGEFFECTSETTINGSSPARKS_H

#include "battledialogeffectsettingsbase.h"

namespace Ui {
class BattleDialogEffectSettingsSparks;
}

class BattleDialogModelEffectSparks;

class BattleDialogEffectSettingsSparks : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettingsSparks(const BattleDialogModelEffectSparks& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsSparks();

    // BattleDialogEffectSettingsBase
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

private:
    Ui::BattleDialogEffectSettingsSparks *ui;
};

#endif // BATTLEDIALOGEFFECTSETTINGSSPARKS_H
