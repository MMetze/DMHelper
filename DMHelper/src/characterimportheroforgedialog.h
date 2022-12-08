#ifndef CHARACTERIMPORTHEROFORGEDIALOG_H
#define CHARACTERIMPORTHEROFORGEDIALOG_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class CharacterImportHeroForgeDialog;
}

class CharacterImportHeroForge;
class CharacterImportHeroForgeData;
class QGridLayout;

class CharacterImportHeroForgeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharacterImportHeroForgeDialog(QWidget *parent = nullptr);
    ~CharacterImportHeroForgeDialog();

protected:
    virtual void showEvent(QShowEvent *event) override;

private slots:
    void importComplete(QList<CharacterImportHeroForgeData*> data);
    void dataReady(CharacterImportHeroForgeData* data);

private:
    void addDataRow(CharacterImportHeroForgeData* data, int row);
    void addData(QImage image, QString dataName, int row, int column);

    Ui::CharacterImportHeroForgeDialog *ui;

    QGridLayout* _iconGrid;
    QButtonGroup _buttonGroup;
    CharacterImportHeroForge* _importer;
};

#endif // CHARACTERIMPORTHEROFORGEDIALOG_H
