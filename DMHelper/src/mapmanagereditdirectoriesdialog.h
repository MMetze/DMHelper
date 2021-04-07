#ifndef MAPMANAGEREDITDIRECTORIESDIALOG_H
#define MAPMANAGEREDITDIRECTORIESDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class MapManagerEditDirectoriesDialog;
}

class MapManagerEditDirectoriesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapManagerEditDirectoriesDialog(QStringList directories,
                                             bool stringsOnly = false,
                                             const QString& dialogTitle = QString(),
                                             const QString& dataName = QString(),
                                             QWidget *parent = nullptr);
    ~MapManagerEditDirectoriesDialog();

    QStringList getDirectories();

protected:
    void addDirectory();
    void addDirectoryString();
    void removeDirectory();

private:
    Ui::MapManagerEditDirectoriesDialog *ui;

    QString _dataName;
};

#endif // MAPMANAGEREDITDIRECTORIESDIALOG_H
