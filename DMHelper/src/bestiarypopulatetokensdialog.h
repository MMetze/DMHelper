#ifndef BESTIARYPOPULATETOKENSDIALOG_H
#define BESTIARYPOPULATETOKENSDIALOG_H

#include <QDialog>
#include <QDir>

namespace Ui {
class BestiaryPopulateTokensDialog;
}

class OptionsContainer;
class TokenEditor;
class DMHWaitingDialog;
class QNetworkAccessManager;
class QNetworkReply;

class BestiaryPopulateTokensDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryPopulateTokensDialog(const OptionsContainer& options, QWidget *parent = nullptr);
    ~BestiaryPopulateTokensDialog();

protected slots:
    void populateTokens();

    void urlRequestFinished(QNetworkReply *reply);
    void imageRequestFinished(QNetworkReply *reply);
    void browseFrame();
    void browseMask();

    void updateImage();

protected:
    void checkNextMonster();
    void postRequest();
    void cancelRequests();

private:
    Ui::BestiaryPopulateTokensDialog *ui;

    QNetworkAccessManager* _manager;
    QNetworkReply* _activeReply;

    DMHWaitingDialog* _waitingDlg;

    TokenEditor* _editor;
    QDir _tokenDir;

    QString _searchString;
    QString _currentMonster;
    QStringList _monsterList;
    int _totalPopulated;
    int _totalMonsters;
};

#endif // BESTIARYPOPULATETOKENSDIALOG_H
