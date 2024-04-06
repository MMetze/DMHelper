#ifndef LAYEREFFECTSETTINGS_H
#define LAYEREFFECTSETTINGS_H

#include <QDialog>

namespace Ui {
class LayerEffectSettings;
}

class LayerEffectSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LayerEffectSettings(QWidget *parent = nullptr);
    ~LayerEffectSettings();

    static const int defaultWidth = 500;
    static const int defaultHeight = 500;
    static const int defaultThickness = 50;
    static const int defaultDirection = 45;
    static const int defaultSpeed = 20;
    static const int defaultMorph = 50;
    static const Qt::GlobalColor defaultColor = Qt::white;

    int effectWidth() const;
    int effectHeight() const;
    int effectThickness() const;
    int effectDirection() const;
    int effectSpeed() const;
    int effectMorph() const;
    QColor effectColor() const;

    static QImage createNoiseImage(const QSize& imageSize, qreal width, qreal height, const QColor& color, qreal thickness);
    static float noise(float x, float y, float z);

signals:
    void effectWidthChanged(int width);
    void effectHeightChanged(int height);
    void effectThicknessChanged(int thickness);
    void effectDirectionChanged(int direction);
    void effectSpeedChanged(int speed);
    void effectMorphChanged(int morph);
    void effectColorChanged(const QColor& color);

public slots:
    void setEffectWidth(int width);
    void setEffectHeight(int height);
    void setEffectThickness(int thickness);
    void setEffectDirection(int direction);
    void setEffectSpeed(int speed);
    void setEffectMorph(int morph);
    void setEffectColor(const QColor& color);

protected slots:
    // QObject
    virtual void timerEvent(QTimerEvent *event) override;

    // QDialog
    virtual void showEvent(QShowEvent *event) override;

    // local
    void handleDialDirectionChanged(int direction);
    void startRedrawTimer();

private:
    int clampTo360(int value);
    static int32_t fastfloor(float fp);
    static uint8_t hash(int32_t i);
    static float grad(int32_t hash, float x, float y, float z);

    Ui::LayerEffectSettings *ui;
    int _timerId;
};

#endif // LAYEREFFECTSETTINGS_H






