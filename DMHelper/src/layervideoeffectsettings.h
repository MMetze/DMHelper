#ifndef LAYERVIDEOEFFECTSETTINGS_H
#define LAYERVIDEOEFFECTSETTINGS_H

#include "layervideoeffect.h"
#include <QDialog>

namespace Ui {
class LayerVideoEffectSettings;
}

class LayerVideoEffectSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LayerVideoEffectSettings(QWidget *parent = nullptr);
    ~LayerVideoEffectSettings();

    LayerVideoEffect::LayerVideoEffectType getEffectType() const;
    bool isColorize() const;

    QColor getTransparentColor() const;
    QColor getColorizeColor() const;

private:
    Ui::LayerVideoEffectSettings *ui;
};

#endif // LAYERVIDEOEFFECTSETTINGS_H
