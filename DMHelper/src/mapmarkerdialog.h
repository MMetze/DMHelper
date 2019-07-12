#ifndef MAPMARKERDIALOG_H
#define MAPMARKERDIALOG_H

#include <QDialog>

namespace Ui {
class MapMarkerDialog;
}

class MapMarkerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapMarkerDialog(QString title, QString description, QWidget *parent = nullptr);
    ~MapMarkerDialog();

    QString getTitle();
    QString getDescription();

private:
    Ui::MapMarkerDialog *ui;
};

#endif // MAPMARKERDIALOG_H
