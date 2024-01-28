#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "optionscontainer.h"
#include "dmconstants.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(OptionsContainer* options, QWidget *parent = nullptr);
    ~OptionsDialog();

    OptionsContainer* getOptions() const;

private slots:
    void browseBestiary();
    void editBestiary();
    void setBestiary(const QString& bestiaryFile);

    void browseSpellbook();
    void editSpellbook();
    void setSpellbook(const QString& spellbookFile);

    void browseQuickReference();
    void editQuickReference();
    void setQuickReference(const QString& quickRefFile);

    void browseCalendar();
    void editCalendar();
    void setCalendar(const QString& calendarFile);

    void browseEquipment();
    void editEquipment();
    void setEquipment(const QString& equipmentFile);

    void browseShops();
    void editShops();
    void setShops(const QString& shopsFile);

    void browseTables();
    void editTables();
    void setTables(const QString& tablesDirectory);

    void handleInitiativeScaleChanged(qreal initiativeScale);

    void browsePointerFile();
    void editPointerFile();
    void setPointerFile(const QString& pointerFile);

    void browseSelectedIcon();
    void editSelectedIcon();
    void setSelectedIcon(const QString& selectedIcon);

    void browseActiveIcon();
    void editActiveIcon();
    void setActiveIcon(const QString& activeIcon);

    void browseCombatantFrame();
    void editCombatantFrame();
    void setCombatantFrame(const QString& combatantFrame);

    void browseCountdownFrame();
    void editCountdownFrame();
    void setCountdownFrame(const QString& countdownFrame);

    void heroForgeTokenEdited();

    void tokenSearchEdited();

    void browseTokenFrame();
    void editTokenFrame();
    void setTokenFrame(const QString& tokenFrame);

    void browseTokenMask();
    void editTokenMask();
    void setTokenMask(const QString& tokenMask);
    void populateTokens();

    void updateFileLocations();
    void resetFileLocations();

private:
    Ui::OptionsDialog *ui;

    OptionsContainer* _options;
};

#endif // OPTIONSDIALOG_H
