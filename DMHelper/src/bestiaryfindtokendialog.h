#ifndef BESTIARYFINDTOKENDIALOG_H
#define BESTIARYFINDTOKENDIALOG_H

#include "tokeneditor.h"
#include <QDialog>

namespace Ui {
class BestiaryFindTokenDialog;
}

class TokenData;
class QGridLayout;
class QNetworkAccessManager;
class QNetworkReply;

class BestiaryFindTokenDialog : public QDialog
{
    Q_OBJECT

public:

    explicit BestiaryFindTokenDialog(const QString& monsterName, const QString& searchString, const OptionsContainer& options, QWidget *parent = nullptr);
    ~BestiaryFindTokenDialog();

    QList<QImage> retrieveSelection(bool decorated = true);

    /*
    bool isBackgroundFill() const;
    QColor getBackgroundFillColor() const;
    bool isTransparent() const;
    QColor getTransparentColor() const;
    int getTransparentLevel() const;
    bool isMaskApplied() const;
    QString getMaskFile() const;
    bool isFrameApplied() const;
    QString getFrameFile() const;
*/

    bool isEditingToken() const;
    TokenEditor* getEditor();

protected slots:
    void urlRequestFinished(QNetworkReply *reply);
    void imageRequestFinished(QNetworkReply *reply);
    void browseFrame();
    void browseMask();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

    void startSearch(const QString& searchString);
    void abortSearches();

    void customizeSearch();
    void updateLayout();
    void updateLayoutImages();
    void clearGrid();
    void rescaleData();
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

    TokenEditor* _editor;

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
