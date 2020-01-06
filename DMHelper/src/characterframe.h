#ifndef CHARACTERFRAME_H
#define CHARACTERFRAME_H

#include <QFrame>

namespace Ui {
class CharacterFrame;
}

class Character;
class QCheckBox;

class CharacterFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CharacterFrame(QWidget *parent = nullptr);
    ~CharacterFrame();

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

    Ui::CharacterFrame *ui;
    Character* _character;
    bool _mouseDown;
    bool _reading;
};

#endif // CHARACTERFRAME_H
