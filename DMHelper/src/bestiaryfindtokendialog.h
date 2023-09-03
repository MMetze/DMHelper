#ifndef BESTIARYFINDTOKENDIALOG_H
#define BESTIARYFINDTOKENDIALOG_H

#include <QDialog>

namespace Ui {
class BestiaryFindTokenDialog;
}

class QGridLayout;
class TokenData;
class QNetworkAccessManager;
class QNetworkReply;

class BestiaryFindTokenDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BestiaryFindTokenDialog(const QString& monsterName, QWidget *parent = nullptr);
    ~BestiaryFindTokenDialog();

protected slots:
    void urlRequestFinished(QNetworkReply *reply);
    void imageRequestFinished(QNetworkReply *reply);

protected:
    void updateLayout();
    void clearGrid();
    TokenData* getDataForReply(QNetworkReply *reply);

private:
    Ui::BestiaryFindTokenDialog *ui;

    QString _monsterName;
    QNetworkAccessManager* _manager;
    QGridLayout* _tokenGrid;
    QList<TokenData*> _tokenList;
};

class TokenData
{
public:
    TokenData(const QString& tokenAddress) : _tokenAddress(tokenAddress), _reply(nullptr) {}

    QString _tokenAddress;
    QNetworkReply* _reply;
    QPixmap _pixmap;
};

#endif // BESTIARYFINDTOKENDIALOG_H
