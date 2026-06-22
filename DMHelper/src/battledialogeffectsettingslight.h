#ifndef BATTLEDIALOGEFFECTSETTINGSLIGHT_H
#define BATTLEDIALOGEFFECTSETTINGSLIGHT_H

#include "battledialogeffectsettingsbase.h"

namespace Ui {
class BattleDialogEffectSettingsLight;
}

class BattleDialogModelEffectLight;

class BattleDialogEffectSettingsLight : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettingsLight(const BattleDialogModelEffectLight& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsLight();

    // BattleDialogEffectSettingsBase
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

private:
    Ui::BattleDialogEffectSettingsLight *ui;
};

#endif // BATTLEDIALOGEFFECTSETTINGSLIGHT_H
