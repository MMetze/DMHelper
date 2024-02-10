#ifndef BATTLEDIALOGEFFECTSETTINGSOBJECTVIDEO_H
#define BATTLEDIALOGEFFECTSETTINGSOBJECTVIDEO_H

#include "battledialogeffectsettingsbase.h"
#include "dmconstants.h"

namespace Ui {
class BattleDialogEffectSettingsObjectVideo;
}

class BattleDialogModelEffectObjectVideo;

class BattleDialogEffectSettingsObjectVideo : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettingsObjectVideo(const BattleDialogModelEffectObjectVideo& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettingsObjectVideo();

    // Base clase implementations
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

    bool isEffectActive() const;
    bool isEffectVisible() const;
    QString getTip() const;
    int getHeightValue() const;
    int getWidthValue() const;
    qreal getRotation() const;

    DMHelper::TransparentType getEffectTransparencyType() const;
    QColor getTransparentColor() const;
    qreal getTransparentTolerance() const;
    bool isColorize() const;
    QColor getColorizeColor() const;

private:
    Ui::BattleDialogEffectSettingsObjectVideo *ui;
    const BattleDialogModelEffectObjectVideo& _effect;
};

#endif // BATTLEDIALOGEFFECTSETTINGSOBJECTVIDEO_H
