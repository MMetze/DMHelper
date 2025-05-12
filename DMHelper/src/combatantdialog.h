#ifndef COMBATANTDIALOG_H
#define COMBATANTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "combatant.h"

class MonsterClassv2;
class LayerScene;
class LayerTokens;

namespace Ui {
class CombatantDialog;
}

class CombatantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CombatantDialog(LayerScene& layerScene, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close, QWidget *parent = nullptr);
    ~CombatantDialog();

    int getCount() const;
    QString getName() const;
    LayerTokens* getLayer() const;
    int getCombatantHitPoints() const;
    bool isRandomInitiative() const;
    bool isSortInitiative() const;
    QString getInitiative() const;
    bool isKnown() const;
    bool isShown() const;
    bool isCustomSize() const;
    QString getSizeFactor() const;
    MonsterClassv2* getMonsterClass() const;
    int getIconIndex() const;
    QString getIconFile() const;

    void writeCombatant(Combatant* combatant);

signals:
    void openMonster(const QString& monsterClass);

public slots:
    // From QDialog
    virtual void accept() override;

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void monsterClassChanged(const QString &text);
    void setIconIndex(int index);
    void updateIcon();
    void previousIcon();
    void selectCustomToken();
    void nextIcon();
    void setHitPointAverageChanged();
    void openMonsterClicked();
    void sizeSelected(int index);

private:
    void fillSizeCombo();

    Ui::CombatantDialog *ui;
    int _iconIndex;
    QString _iconFile;

};

#endif // COMBATANTDIALOG_H
