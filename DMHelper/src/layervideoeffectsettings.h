#ifndef LAYERVIDEOEFFECTSETTINGS_H
#define LAYERVIDEOEFFECTSETTINGS_H

#include "tokeneditor.h"
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

    DMHelper::TransparentType getEffectType() const;
    QColor getTransparentColor() const;
    qreal getTransparentTolerance() const;
    bool isColorize() const;
    QColor getColorizeColor() const;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void setEffectType(DMHelper::TransparentType effectType);
    void setTransparentColor(const QColor& transparentColor);
    void setTransparentTolerance(qreal transparentTolerance);
    void setColorize(bool colorize);
    void setColorizeColor(const QColor& colorizeColor);
    void setPreviewImage(const QImage& image);

private slots:
    void handleButtonChanged(int id, bool checked);
    void handleValueChanged(int value);
    void setEditorSource();
    void updatePreview();

private:
    Ui::LayerVideoEffectSettings *ui;

    QImage _previewImage;
    TokenEditor* _editor;
};

#endif // LAYERVIDEOEFFECTSETTINGS_H
