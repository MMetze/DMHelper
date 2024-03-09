#ifndef BATTLEDIALOGEFFECTSETTINGS_H
#define BATTLEDIALOGEFFECTSETTINGS_H

#include "battledialogeffectsettingsbase.h"

namespace Ui {
class BattleDialogEffectSettings;
}

class BattleDialogEffectSettings : public BattleDialogEffectSettingsBase
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent = nullptr);
    virtual ~BattleDialogEffectSettings();

    // Base clase implementations
    virtual void mergeValuesToSettings(BattleDialogModelEffect& effect) override;
    virtual void copyValuesFromSettings(BattleDialogModelEffect& effect) override;

    bool isEffectActive() const;
    bool isEffectVisible() const;
    QString getTip() const;
    int getSizeValue() const;
    int getWidthValue() const;
    qreal getRotation() const;
    QColor getColor() const;
    int getAlpha() const;

public slots:
    void setSizeLabel(const QString& sizeLabel);
    void setShowActive(bool show);
    void setShowWidth(bool show);
    void setShowColor(bool show);

private slots:
    void selectNewColor();

private:
    void setButtonColor(const QColor& color);

    Ui::BattleDialogEffectSettings *ui;

    QColor _color;
};

#endif // BATTLEDIALOGEFFECTSETTINGS_H
