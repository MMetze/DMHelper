#ifndef CHARACTERFRAME_H
#define CHARACTERFRAME_H

#include "campaignobjectframe.h"

namespace Ui {
class CharacterFrame;
}

class Character;
class QCheckBox;

class CharacterFrame : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit CharacterFrame(QWidget *parent = nullptr);
    ~CharacterFrame();

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setCharacter(Character* character);

signals:
    void publishCharacterImage(QImage img);
    void characterChanged();

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
    void syncDndBeyond();
    void openExpertiseDialog();

private:
    void loadCharacterImage();
    void updateCheckboxName(QCheckBox* chk, int abilityMod, int proficiencyBonus, bool expertise, bool halfProficiency);
    void enableDndBeyondSync(bool enabled);

    void connectChanged(bool makeConnection);

    Ui::CharacterFrame *ui;
    Character* _character;
    bool _mouseDown;
    bool _reading;
    int _rotation;
};

#endif // CHARACTERFRAME_H
