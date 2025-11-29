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

public slots:
    void setMapColor(const QColor& color);
    void setMapSize(const QSize& size);
    void enableSizeEditing(bool enable);

private:
    Ui::MapBlankDialog *ui;
};

#endif // MAPBLANKDIALOG_H
