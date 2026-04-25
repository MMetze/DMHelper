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
    struct SparksPreset
    {
        QString name;
        int size;
        QColor color;
        int opacity;
        int particleCount;
        qreal glowRadius;
        qreal glowOpacity;
        bool arcFalloff;
        bool fadeDistance;
        qreal sparkSpeed;
        qreal windDirection;
        qreal windStrength;
    };

    void initializePresets();
    void applyPreset(int presetIndex);

    Ui::BattleDialogEffectSettingsSparks *ui;
};

#endif // BATTLEDIALOGEFFECTSETTINGSSPARKS_H
