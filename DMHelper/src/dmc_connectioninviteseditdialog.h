#ifndef DMC_CONNECTIONINVITESEDITDIALOG_H
#define DMC_CONNECTIONINVITESEDITDIALOG_H

#include <QDialog>

namespace Ui {
class DMC_ConnectionInvitesEditDialog;
}

class DMC_OptionsContainer;
class QTableWidgetItem;

class DMC_ConnectionInvitesEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DMC_ConnectionInvitesEditDialog(DMC_OptionsContainer& options, QWidget *parent = nullptr);
    ~DMC_ConnectionInvitesEditDialog();

private slots:
    void addInvite();
    void renameInvite();
    void removeInvite();

    void inviteChanged(QTableWidgetItem *item);

private:
    void populateInvites();

    Ui::DMC_ConnectionInvitesEditDialog *ui;
    DMC_OptionsContainer& _options;
};

#endif // DMC_CONNECTIONINVITESEDITDIALOG_H
