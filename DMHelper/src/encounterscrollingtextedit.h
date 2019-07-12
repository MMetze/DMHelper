#ifndef ENCOUNTERSCROLLINGTEXTEDIT_H
#define ENCOUNTERSCROLLINGTEXTEDIT_H

#include <QFrame>

namespace Ui {
class EncounterScrollingTextEdit;
}

class EncounterScrollingText;
class QGraphicsScene;
class QGraphicsTextItem;

class EncounterScrollingTextEdit : public QFrame
{
    Q_OBJECT

public:
    explicit EncounterScrollingTextEdit(QWidget *parent = nullptr);
    ~EncounterScrollingTextEdit();

    EncounterScrollingText* getScrollingText() const;
    void setScrollingText(EncounterScrollingText* scrollingText);
    void unsetScrollingText(EncounterScrollingText* scrollingText);

signals:
    void scrollSpeedChanged(double scrollSpeed);
    void imgFileChanged(const QString& imgFile);
    void textChanged(const QString& newText);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(int alignment);
    void imageWidthChanged(int imageWidth);
    void colorChanged(QColor color);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void setPlainText();
    void setFontSize();
    void setAlignment();
    void setColor();
    void setTextFont();
    void browseImageFile();

    void createScene();
    void updatePreviewText(const QString& newText);
    void updatePreviewFont();

    void runAnimation();

private:

    Qt::AlignmentFlag getAlignment();

    Ui::EncounterScrollingTextEdit *ui;

    EncounterScrollingText* _scrollingText;
    QGraphicsScene* _textScene;
    QGraphicsTextItem* _textItem;
    int _backgroundWidth;
};

#endif // ENCOUNTERSCROLLINGTEXTEDIT_H
