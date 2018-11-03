#ifndef CHASERWIDGET_H
#define CHASERWIDGET_H

#include <QFrame>

class QLineEdit;
class QCheckBox;
class QPushButton;
class ChaserCombatant;

class ChaserWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ChaserWidget(ChaserCombatant* chaser, QWidget *parent = 0);

    ChaserCombatant* getChaser() const;

    void resetStyleSheet();

signals:
    void positionChanged(int position);
    void initiativeChanged(int initiative);
    void exhaustionChanged(int exhaustion);

public slots:
    void readData();

    void positionEdited();
    void initiativeEdited();
    void exhaustionClicked();
    void dashEdited();
    void speedEdited();

protected:
    void buildWidget();

    QLineEdit* nameEdit;
    QLineEdit* speedEdit;
    QLineEdit* dashEdit;
    QLineEdit* positionEdit;
    QLineEdit* initiativeEdit;
    QCheckBox* exhaustion1;
    QCheckBox* exhaustion2;
    QCheckBox* exhaustion3;
    QCheckBox* exhaustion4;
    QCheckBox* exhaustion5;

    ChaserCombatant* _chaser;
};

#endif // CHASERWIDGET_H
