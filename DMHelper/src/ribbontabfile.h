#ifndef RIBBONTABFILE_H
#define RIBBONTABFILE_H

#include <QFrame>
#include <QMenu>

namespace Ui {
class RibbonTabFile;
}

class RibbonTabFile : public QFrame
{
    Q_OBJECT

public:
    explicit RibbonTabFile(QWidget *parent = nullptr);
    ~RibbonTabFile();

    QMenu* getMRUMenu() const;

signals:
    void newClicked();
    void openClicked();
    void saveClicked();
    void saveAsClicked();

    void optionsClicked();
    void closeClicked();

private slots:
    void showMRUMenu();
    void hideMRUMenu();

private:
    Ui::RibbonTabFile *ui;
    QMenu* _mruMenu;
};

#endif // RIBBONTABFILE_H
