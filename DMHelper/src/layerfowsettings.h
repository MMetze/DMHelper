#ifndef LAYERFOWSETTINGS_H
#define LAYERFOWSETTINGS_H

#include <QDialog>

namespace Ui {
class LayerFowSettings;
}

class LayerFowSettings : public QDialog
{
    Q_OBJECT

public:
    explicit LayerFowSettings(QWidget *parent = nullptr);
    ~LayerFowSettings();

    QColor fowColor() const;
    QString fowTextureFile() const;
    int fowScale() const;

signals:
    void fowColorChanged(const QColor& color);
    void fowTextureFileChanged(const QString& textureFile);
    void fowScaleChanged(int scale);

public slots:
    void setFowColor(const QColor& color);
    void setFowTextureFile(const QString& textureFile);
    void setFowScale(int scale);

protected:
    virtual bool eventFilter(QObject *o, QEvent *e) override;

    void handleColorChanged(const QColor& color);
    void handleTextureFileChanged();
    void handleTextureBrowseClicked();
    void handleScaleChanged(int scale);

    void updatePreview();

private:
    Ui::LayerFowSettings *ui;

    QPixmap _texturePixmap;
};

#endif // LAYERFOWSETTINGS_H
