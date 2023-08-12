#ifndef CHARACTERIMPORTHEROFORGEDIALOG_H
#define CHARACTERIMPORTHEROFORGEDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QPushButton>

namespace Ui {
class CharacterImportHeroForgeDialog;
}

class CharacterImportHeroForge;
class CharacterImportHeroForgeData;
class QGridLayout;
class DMHWaitingDialog;

class CharacterImportHeroForgeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharacterImportHeroForgeDialog(const QString& token, QWidget *parent = nullptr);
    ~CharacterImportHeroForgeDialog();

    QImage getSelectedImage() const;
    QString getSelectedName() const;

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
    DMHWaitingDialog* _waitingDlg;
    QString _token;

};


class HeroForgeButton : public QPushButton
{
    Q_OBJECT

public:
    explicit HeroForgeButton(QImage image, const QString& dataName, QWidget *parent = nullptr);

    QImage getImage() const;
    QString getDataName() const;

signals:
    void selectButton();

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

private:
    QImage _image;
    QString _dataName;
};


#endif // CHARACTERIMPORTHEROFORGEDIALOG_H
