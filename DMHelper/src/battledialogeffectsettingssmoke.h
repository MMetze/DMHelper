#ifndef BATTLEDIALOGEFFECTSETTINGSSMOKE_H
#define BATTLEDIALOGEFFECTSETTINGSSMOKE_H

#include "battledialogeffectsettingsbase.h"

namespace Ui {
class BattleDialogEffectSettingsSmoke;
}

class BattleDialogModelEffectSmoke;

class BattleDialogEffectSettingsSmoke : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettingsSmoke(const BattleDialogModelEffectSmoke& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsSmoke();

    // BattleDialogEffectSettingsBase
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

private:
    Ui::BattleDialogEffectSettingsSmoke *ui;
};

#endif // BATTLEDIALOGEFFECTSETTINGSSMOKE_H
