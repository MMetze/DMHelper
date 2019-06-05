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
    explicit BattleDialogEffectSettings(int sizeval, qreal rotation, QColor color, QString tip, QWidget *parent = 0);
    explicit BattleDialogEffectSettings(const BattleDialogModelEffect& effect, QWidget *parent = 0);
    ~BattleDialogEffectSettings();

    QString getTip() const;
    int getSizeValue() const;
    qreal getRotation() const;
    QColor getColor() const;
    int getAlpha() const;

    void copyValues(BattleDialogModelEffect& effect);

private slots:
    void selectNewColor();

private:
    void setButtonColor(QColor color);

    Ui::BattleDialogEffectSettings *ui;

    QColor _color;
};

#endif // BATTLEDIALOGEFFECTSETTINGS_H
