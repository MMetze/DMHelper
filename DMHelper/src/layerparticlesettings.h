#ifndef LAYERPARTICLESETTINGS_H
#define LAYERPARTICLESETTINGS_H

#include <QDialog>
#include <QColor>

namespace Ui {
class LayerParticleSettings;
}

class LayerParticleSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LayerParticleSettings(QWidget *parent = nullptr);
    ~LayerParticleSettings();

    int particleCount() const;
    int rainSpeed() const;
    int rainDirection() const;
    int rainAngle() const;
    QColor rainColor() const;
    int rainLength() const;
    int rainOpacity() const;
    int rainWidth() const;
    int rainMovement() const;

signals:
    void particleCountChanged(int count);
    void rainSpeedChanged(int speed);
    void rainDirectionChanged(int direction);
    void rainAngleChanged(int angle);
    void rainColorChanged(const QColor& color);
    void rainLengthChanged(int length);
    void rainOpacityChanged(int opacity);
    void rainWidthChanged(int width);
    void rainMovementChanged(int movement);

public slots:
    void setParticleCount(int count);
    void setRainSpeed(int speed);
    void setRainDirection(int direction);
    void setRainAngle(int angle);
    void setRainColor(const QColor& color);
    void setRainLength(int length);
    void setRainOpacity(int opacity);
    void setRainWidth(int width);
    void setRainMovement(int movement);
    void applyPreset(int index);

private:
    Ui::LayerParticleSettings *ui;
};

#endif // LAYERPARTICLESETTINGS_H
