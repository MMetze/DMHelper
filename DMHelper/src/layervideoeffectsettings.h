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
    QColor getTransparentColor() const;
    qreal getTransparentTolerance() const;
    bool isColorize() const;
    QColor getColorizeColor() const;

public slots:
    void setEffectType(LayerVideoEffect::LayerVideoEffectType effectType);
    void setTransparentColor(const QColor& transparentColor);
    void setTransparentTolerance(qreal transparentTolerance);
    void setColorize(bool colorize);
    void setColorizeColor(const QColor& colorizeColor);

private:
    Ui::LayerVideoEffectSettings *ui;
};

#endif // LAYERVIDEOEFFECTSETTINGS_H
