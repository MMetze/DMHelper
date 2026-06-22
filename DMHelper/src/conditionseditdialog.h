#ifndef CONDITIONSEDITDIALOG_H
#define CONDITIONSEDITDIALOG_H

#include <QDialog>
#include <QHash>
#include <QStringList>

namespace Ui {
class ConditionsEditDialog;
}

class QPushButton;
class QLabel;
class QFrame;

class ConditionsEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConditionsEditDialog(QWidget *parent = nullptr);
    ~ConditionsEditDialog();

    void setConditionList(const QStringList& conditions);
    QStringList getConditionList() const;

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    struct ConditionEntry
    {
        QString id;
        QString group;
        QPushButton* button;
        QLabel* label;
    };

    void populateConditions();
    void setConditionTooltip(QPushButton& button, const QString& conditionId);

    Ui::ConditionsEditDialog* ui;
    QList<ConditionEntry> _entries;
    QHash<QString, QPushButton*> _groupButtons;
    QHash<QString, QFrame*> _groupFrames;
};

#endif // CONDITIONSEDITDIALOG_H
