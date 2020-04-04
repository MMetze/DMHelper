#ifndef ENCOUNTERTEXTEDIT_H
#define ENCOUNTERTEXTEDIT_H

#include <QTextBrowser>

class EncounterText;

class EncounterTextEdit : public QTextBrowser
{
    Q_OBJECT
public:
    explicit EncounterTextEdit(QWidget *parent = 0);

    void setKeys(QList<QString> keys);
    QList<QString> keys();

    EncounterText* getEncounter() const;
    void setEncounter(EncounterText* encounter);

signals:

public slots:
    void clear();

protected:
    virtual void keyPressEvent(QKeyEvent * e);

    QList<QString> _keys;
    EncounterText* _encounter;
};

#endif // ENCOUNTERTEXTEDIT_H
