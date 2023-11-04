#ifndef TOKENEDITDIALOG_H
#define TOKENEDITDIALOG_H

#include <QDialog>

namespace Ui {
class TokenEditDialog;
}

class TokenEditDialog : public QDialog
{
    Q_OBJECT

public:

    enum TokenDetailMode
    {
        TokenDetailMode_Original = 0,
        TokenDetailMode_TransparentColor,
        TokenDetailMode_FrameAndMask
    };

    explicit TokenEditDialog(const QString& tokenFilename, TokenDetailMode mode, const QColor& background, int backgroundLevel, const QString& frameFile, const QString& maskFile, bool fill, QColor fillColor, QWidget *parent = nullptr);
    ~TokenEditDialog();

    QImage getFinalImage();

    TokenDetailMode getTokenDetailMode() const;
    QColor getTokenBackgroundColor() const;
    int getTokenBackgroundLevel() const;
    QString getTokenFrameFile() const;
    QString getTokenMaskFile() const;

protected slots:
    void updateImage();
    void browseFrame();
    void browseMask();
    void updateFrameMaskImages();

private:
    bool fuzzyColorMatch(QRgb first, QRgb second);

    Ui::TokenEditDialog *ui;

    QImage _maskImage;
    QImage _frameImage;

    QImage _sourceImage;
    QImage _finalImage;
};

#endif // TOKENEDITDIALOG_H
