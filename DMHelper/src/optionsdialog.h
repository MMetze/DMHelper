#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "optionscontainer.h"

namespace Ui {
class OptionsDialog;
}

class Campaign;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(OptionsContainer* options, Campaign* campaign, QWidget *parent = nullptr);
    ~OptionsDialog();

    OptionsContainer* getOptions() const;
    void applyCampaignChanges();

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

    void browseRuleset();
    void editRuleset();
    void setRuleset(const QString& rulesetFile);

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

    void browseCharacterDataFile();
    void editCharacterDataFile();
    void setCharacterDataFile(const QString& characterDataFile);

    void browseCharacterUIFile();
    void editCharacterUIFile();
    void setCharacterUIFile(const QString& characterUIFile);

    void browseBestiaryFile();
    void editBestiaryFile();
    void setBestiaryFile(const QString& bestiaryFile);

    void browseMonsterDataFile();
    void editMonsterDataFile();
    void setMonsterDataFile(const QString& monsterDataFile);

    void browseMonsterUIFile();
    void editMonsterUIFile();
    void setMonsterUIFile(const QString& monsterUIFile);

private:
    Ui::OptionsDialog *ui;

    OptionsContainer* _options;
    Campaign* _campaign;
};

#endif // OPTIONSDIALOG_H
