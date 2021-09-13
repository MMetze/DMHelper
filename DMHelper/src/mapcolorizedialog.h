#ifndef MAPCOLORIZEDIALOG_H
#define MAPCOLORIZEDIALOG_H

#include "mapcolorizefilter.h"
#include <QDialog>

namespace Ui {
class MapColorizeDialog;
}

class MapColorizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapColorizeDialog(QImage originalImage, const MapColorizeFilter& filter, QWidget *parent = nullptr);
    ~MapColorizeDialog();

    MapColorizeFilter getFilter() const;

protected slots:
    void applyFilter();
    void resetFilter();
    void presetSelected(const QString &text);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

    void connectControls(bool connectMe);
    void checkScaledOriginal();
    void readFilter();

private:
    Ui::MapColorizeDialog *ui;

    QImage _originalImage;
    QImage _scaledOriginal;
    MapColorizeFilter _filter;
};

#endif // MAPCOLORIZEDIALOG_H
