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

    enum TokenDetailMode
    {
        TokenDetailMode_Original = 0,
        TokenDetailMode_TransparentColor,
        TokenDetailMode_FrameAndMask
    };

    explicit BestiaryFindTokenDialog(const QString& monsterName, const QString& searchString, TokenDetailMode mode, const QColor& background, int backgroundLevel, const QString& frameFile, const QString& maskFile, QWidget *parent = nullptr);
    ~BestiaryFindTokenDialog();

    QList<QImage> retrieveSelection();

    TokenDetailMode getTokenDetailMode() const;
    QColor getTokenBackgroundColor() const;
    int getTokenBackgroundLevel() const;
    QString getTokenFrameFile() const;
    QString getTokenMaskFile() const;

protected slots:
    void urlRequestFinished(QNetworkReply *reply);
    void imageRequestFinished(QNetworkReply *reply);
    void browseFrame();
    void browseMask();

protected:
    void startSearch(const QString& searchString);
    void abortSearches();

    void customizeSearch();
    void updateLayout();
    void updateLayoutImages();
    void updateFrameMaskImages();
    void clearGrid();
    TokenData* getDataForReply(QNetworkReply *reply);
    QPixmap decoratePixmap(QPixmap pixmap, const QColor& background);
    QImage decorateFullImage(QPixmap pixmap, const QColor& background);
    bool fuzzyColorMatch(QRgb first, QRgb second);

private:
    Ui::BestiaryFindTokenDialog *ui;

    QString _monsterName;
    QString _searchString;

    QNetworkAccessManager* _manager;
    QNetworkReply* _urlReply;

    QImage _maskImage;
    QImage _frameImage;

    QGridLayout* _tokenGrid;
    QList<TokenData*> _tokenList;
};

class TokenData
{
public:
    TokenData(const QString& tokenAddress) : _tokenAddress(tokenAddress), _reply(nullptr), _button(nullptr), _background(Qt::white) {}

    QString _tokenAddress;
    QNetworkReply* _reply;
    QPixmap _pixmap;
    QPixmap _scaledPixmap;
    QPushButton* _button;
    QColor _background;
};

#endif // BESTIARYFINDTOKENDIALOG_H