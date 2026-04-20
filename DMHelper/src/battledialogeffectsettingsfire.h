#ifndef BATTLEDIALOGEFFECTSETTINGSFIRE_H
#define BATTLEDIALOGEFFECTSETTINGSFIRE_H

#include "battledialogeffectsettingsbase.h"

namespace Ui {
class BattleDialogEffectSettingsFire;
}

class BattleDialogModelEffectFire;

class BattleDialogEffectSettingsFire : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettingsFire(const BattleDialogModelEffectFire& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsFire();

    // BattleDialogEffectSettingsBase
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

private:
    Ui::BattleDialogEffectSettingsFire *ui;
};

#endif // BATTLEDIALOGEFFECTSETTINGSFIRE_H
