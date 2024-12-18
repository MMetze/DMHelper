#ifndef BESTIARYDIALOG_H
#define BESTIARYDIALOG_H

#include <QDialog>
#include "bestiaryfindtokendialog.h"

class MonsterClassv2;
class MonsterAction;
class Bestiary;
class TokenEditor;
class OptionsContainer;
class QVBoxLayout;

namespace Ui {
class BestiaryDialog;
}

class BestiaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryDialog(QWidget *parent = nullptr);
    ~BestiaryDialog();

    MonsterClassv2* getMonster() const;

    void setOptions(OptionsContainer* options);

signals:
    void monsterChanged();
    void publishMonsterImage(QImage img, const QColor& color);
    void dialogClosed();

public slots:
    void setMonster(MonsterClassv2* monster, bool edit = true);
    void setMonster(const QString& monsterName, bool edit = true);
    void createNewMonster();
    void deleteCurrentMonster();

    void previousMonster();
    void nextMonster();

    void dataChanged();

protected slots:
    void hitDiceChanged();
    void abilityChanged();
    void updateAbilityMods();
    void monsterRenamed();
    void handlePublishButton();

    void handlePreviousToken();
    void handleAddToken();
    void handleEditToken();
    void handleSearchToken();
    void handleReloadImage();
    void handleClearImage();
    void handleNextToken();

    void addAction();
    void deleteAction(const MonsterAction& action);
    void addLegendaryAction();
    void deleteLegendaryAction(const MonsterAction& action);
    void addSpecialAbility();
    void deleteSpecialAbility(const MonsterAction& action);
    void addReaction();
    void deleteReaction(const MonsterAction& action);

    void handleChallengeEdited();
    void handleEditedData();

protected:
    // From QWidget
    virtual void closeEvent(QCloseEvent * event);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void showEvent(QShowEvent * event);
    virtual void hideEvent(QHideEvent * event);
    virtual void focusOutEvent(QFocusEvent * event);

    void createTokenFiles(BestiaryFindTokenDialog* dialog);

private:
    QString selectToken();
    void setTokenIndex(int index);
    void loadMonsterImage();
    void storeMonsterData();

    void clearActionWidgets();
    void clearWidget(QWidget* widget);

    void interpretChallengeRating(const QString& inputCR);

    Ui::BestiaryDialog *ui;
    QWidget* _actionsWidget;
    QWidget* _legendaryActionsWidget;
    QWidget* _specialAbilitiesWidget;
    QWidget* _reactionsWidget;

    OptionsContainer* _options;
    MonsterClassv2* _monster;
    int _currentToken;
    bool _edit;
    bool _mouseDown;

    // Token search data
    QString _searchString;
};

#endif // BESTIARYDIALOG_H
