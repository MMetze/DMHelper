#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "optionscontainer.h"
#include "dmconstants.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(OptionsContainer* options, QWidget *parent = nullptr);
    ~OptionsDialog();

    OptionsContainer* getOptions() const;

private slots:
    void browseBestiary();
    void editBestiary();
    void setBestiary(const QString& bestiaryFile);

    void browseQuickReference();
    void editQuickReference();
    void setQuickReference(const QString& quickRefFile);

    void browseCalendar();
    void editCalendar();
    void setCalendar(const QString& calendarFile);

    void browseEquipment();
    void editEquipment();
    void setEquipment(const QString& equipmentFile);

    void browseShops();
    void editShops();
    void setShops(const QString& shopsFile);

    void browseTables();
    void editTables();
    void setTables(const QString& tablesDirectory);

    void browsePointerFile();
    void editPointerFile();
    void setPointerFile(const QString& pointerFile);

    void updateFileLocations();
    void resetFileLocations();

private:
    Ui::OptionsDialog *ui;

    OptionsContainer* _options;
};

#endif // OPTIONSDIALOG_H
