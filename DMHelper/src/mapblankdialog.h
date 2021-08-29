#ifndef MAPBLANKDIALOG_H
#define MAPBLANKDIALOG_H

#include <QDialog>

namespace Ui {
class MapBlankDialog;
}

class MapBlankDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapBlankDialog(QWidget *parent = nullptr);
    ~MapBlankDialog();

    QColor getMapColor() const;
    int getMapWidth() const;
    int getMapHeight() const;
    QSize getMapSize() const;

private:
    Ui::MapBlankDialog *ui;
};

#endif // MAPBLANKDIALOG_H
