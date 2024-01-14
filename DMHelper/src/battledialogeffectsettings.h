#ifndef BATTLEDIALOGEFFECTSETTINGS_H
#define BATTLEDIALOGEFFECTSETTINGS_H

#include <QDialog>

namespace Ui {
class BattleDialogEffectSettings;
}

class BattleDialogModelEffect;

class BattleDialogEffectSettings : public QDialog
{
    Q_OBJECT

public:
    explicit BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent = nullptr);
    ~BattleDialogEffectSettings();

    bool isEffectActive() const;
    bool isEffectVisible() const;
    QString getTip() const;
    int getSizeValue() const;
    int getWidthValue() const;
    qreal getRotation() const;
    QColor getColor() const;
    int getAlpha() const;

    void mergeValuesToSettings(BattleDialogModelEffect& effect);
    void copyValuesFromSettings(BattleDialogModelEffect& effect);

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
