#ifndef TOKENEDITDIALOG_H
#define TOKENEDITDIALOG_H

#include "tokeneditor.h"
#include <QDialog>

namespace Ui {
class TokenEditDialog;
}

class TokenEditDialog : public QDialog
{
    Q_OBJECT

public:

    explicit TokenEditDialog(const QString& tokenFilename = QString(), bool backgroundFill = false, const QColor& backgroundFillColor = Qt::white, bool transparent = false, const QColor& transparentColor = Qt::white, int transparentLevel = TokenEditor::TRANSPARENT_LEVEL_DEFAULT, bool maskApplied = false, const QString& maskFile = QString(), bool frameApplied = false, const QString& frameFile = QString(), qreal zoom = 1.0, const QPoint& offset = QPoint(), bool browsable = true, QWidget *parent = nullptr);
    explicit TokenEditDialog(const QImage& sourceImage, bool backgroundFill = false, const QColor& backgroundFillColor = Qt::white, bool transparent = false, const QColor& transparentColor = Qt::white, int transparentLevel = TokenEditor::TRANSPARENT_LEVEL_DEFAULT, bool maskApplied = false, const QString& maskFile = QString(), bool frameApplied = false, const QString& frameFile = QString(), qreal zoom = 1.0, const QPoint& offset = QPoint(), bool browsable = true, QWidget *parent = nullptr);
    ~TokenEditDialog();

    void setSourceImage(const QImage& sourceImage);
    void setBackgroundFillColor(const QColor& color);
    QImage getFinalImage();

public slots:
    void updateImage();

protected:
    virtual bool eventFilter(QObject *o, QEvent *e) override;

protected slots:
    void zoomIn();
    void zoomOut();
    void zoomReset();

    void browseImage();
    void browseFrame();
    void browseMask();

private:
    void initialize(bool backgroundFill, const QColor& backgroundFillColor, bool transparent, const QColor& transparentColor, int transparentLevel, bool maskApplied, const QString& maskFile, bool frameApplied, const QString& frameFile, qreal zoom, const QPoint& offset, bool browsable);

    Ui::TokenEditDialog *ui;

    TokenEditor* _editor;
    QPointF _mouseDownPos;
};

#endif // TOKENEDITDIALOG_H
