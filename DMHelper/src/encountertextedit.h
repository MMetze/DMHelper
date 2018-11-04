#ifndef ENCOUNTERTEXTEDIT_H
#define ENCOUNTERTEXTEDIT_H

#include <QFrame>

namespace Ui {
class EncounterTextEdit;
}

class EncounterText;

class EncounterTextEdit : public QFrame
{
    Q_OBJECT

public:
    explicit EncounterTextEdit(QWidget *parent = 0);
    ~EncounterTextEdit();

    void setKeys(QList<QString> keys);
    QList<QString> keys();

    EncounterText* getEncounter() const;
    void setEncounter(EncounterText* encounter);

    QString toHtml() const;
    QString toPlainText() const;

    virtual bool eventFilter(QObject *watched, QEvent *event);

signals:
    void textChanged();
    void anchorClicked(const QUrl &link);

public slots:
    void clear();
    void setHtml(const QString &text);
    void setPlainText(const QString &text);

protected:
    Ui::EncounterTextEdit *ui;

    QList<QString> _keys;
    EncounterText* _encounter;
};

#endif // ENCOUNTERTEXTEDIT_H
