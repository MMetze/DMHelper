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
    explicit CharacterFrame(QWidget *parent = 0);
    ~CharacterFrame();

    void setCharacter(Character* character);

signals:
    void publishCharacterImage(QImage img, QColor color);

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

private:
    void loadCharacterImage();
    void updateCheckboxName(QCheckBox* chk, int abilityMod, int proficiencyBonus);

    Ui::CharacterFrame *ui;
    Character* _character;
    bool _mouseDown;
};

#endif // CHARACTERFRAME_H
