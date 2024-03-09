#ifndef CHARACTERFRAME_H
#define CHARACTERFRAME_H

#include "campaignobjectframe.h"
#include "character.h"

namespace Ui {
class CharacterFrame;
}

class SpellSlotRadioButton;
class OptionsContainer;
class QCheckBox;
class QGridLayout;
class QHBoxLayout;
class QLineEdit;

class CharacterFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit CharacterFrame(OptionsContainer* options, QWidget *parent = nullptr);
    ~CharacterFrame();

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
    virtual void deactivateObject() override;

    void setCharacter(Character* character);
    void setHeroForgeToken(const QString& token);

signals:
    void publishCharacterImage(QImage img);
    void characterChanged();
    void spellSelected(const QString& spellName);
    void heroForgeTokenChanged(const QString& token);

public slots:
    void calculateMods();
    void clear();

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

protected:
    virtual void mousePressEvent(QMouseEvent * event) override;
    virtual void mouseReleaseEvent(QMouseEvent * event) override;

private slots:
    void readCharacterData();
    void writeCharacterData();
    void updateCharacterName();
    void handlePublishClicked();
    void editCharacterIcon();
    void syncDndBeyond();
    void importHeroForge();
    void openExpertiseDialog();
    void editConditions();
    void clearConditions();
    void updateConditionLayout();
    void clearConditionGrid();
    void addCondition(Combatant::Condition condition);
    void addAction();
    void deleteAction(const MonsterAction& action);
    void spellSlotChanged(int level, int slot, bool checked);
    void editLevelSlots(int level);
    void addSpellLevel();
    void pactLevelChanged();
    void spellAnchorClicked(const QUrl &link);

private:
    void loadCharacterImage();
    void updateCheckboxName(QCheckBox* chk, Combatant::Skills skill);
    void enableDndBeyondSync(bool enabled);

    void connectChanged(bool makeConnection);
    void readSpellSlots();
    void updateSpellSlots();
    SpellSlotRadioButton* createRadioButton(int level, int slot, bool checked);
    void clearLayout(QLayout* layout);

    Ui::CharacterFrame *ui;
    OptionsContainer* _options;
    Character* _character;
    bool _mouseDown;
    bool _reading;
    int _rotation;
    QString _heroForgeToken;
    QGridLayout* _conditionGrid;
    QHBoxLayout* _pactSlots;
    QLineEdit* _edtPactLevel;
};

#endif // CHARACTERFRAME_H
