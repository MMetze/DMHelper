#ifndef LAYERDRAWTOOLDIALOG_H
#define LAYERDRAWTOOLDIALOG_H

#include "dmconstants.h"
#include <QDialog>

namespace Ui {
class LayerDrawToolDialog;
}

class LayerDrawToolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LayerDrawToolDialog(QWidget *parent = nullptr);
    ~LayerDrawToolDialog();

    DMHelper::DrawToolType getToolType() const;
    Qt::PenStyle getToolLineType() const;
    QColor getToolLineColor() const;
    int getToolLineWidth() const;
    QColor getToolFillColor() const;
    bool isToolFilled() const;
    QString getToolFontFamily() const;
    int getToolFontSize() const;

signals:
    void toolTypeChanged(DMHelper::DrawToolType toolType);

private slots:
    void handleLineTypeTriggered(QAction* action);
    void handleToolChanged();

private:
    void updateControlStates();

    Ui::LayerDrawToolDialog *ui;
    Qt::PenStyle _currentLineType;
};

class LayerDrawToolDialog_LineTypeAction : public QAction
{
    Q_OBJECT

public:
    explicit LayerDrawToolDialog_LineTypeAction(const QIcon &icon, const QString &text, int lineType, QObject *parent = nullptr);
    virtual ~LayerDrawToolDialog_LineTypeAction() override;

    int getLineType() const;

protected:
    int _lineType;
};


#endif // LAYERDRAWTOOLDIALOG_H
