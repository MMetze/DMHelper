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
    void publishCharacterImage(QImage img, QColor color);
    void characterChanged();

public slots:
    void calculateMods();
    void clear();

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

private slots:
    void readCharacterData();
    void writeCharacterData();
    void handlePublishClicked();
    void syncDndBeyond();
    void openExpertiseDialog();

private:
    void loadCharacterImage();
    void updateCheckboxName(QCheckBox* chk, int abilityMod, int proficiencyBonus, bool expertise, bool halfProficiency);
    void enableDndBeyondSync(bool enabled);

    Ui::CharacterFrame *ui;
    Character* _character;
    bool _mouseDown;
    bool _reading;
};

#endif // CHARACTERFRAME_H
