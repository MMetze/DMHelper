#ifndef ENCOUNTERSCROLLINGTEXTEDIT_H
#define ENCOUNTERSCROLLINGTEXTEDIT_H

#include <QFrame>
#include <QElapsedTimer>

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
    virtual ~EncounterScrollingTextEdit() override;

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

    void animationStarted(QColor color);
    void animateImage(QImage img);
    void showPublishWindow();

public slots:
    void targetResized(const QSize& newSize);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

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
    void startPublishTimer();
    void stopPublishTimer();

    void prepareImages();

private:

    Qt::AlignmentFlag getAlignment();

    Ui::EncounterScrollingTextEdit *ui;

    EncounterScrollingText* _scrollingText;
    QGraphicsScene* _textScene;
    QGraphicsTextItem* _textItem;
    int _backgroundWidth;
    QImage _backgroundImg;
    QImage _prescaledImg;
    QImage _textImg;
    QPointF _textPos;
    QSize _targetSize;
    QElapsedTimer _elapsed;
    int _timerId;
};

#endif // ENCOUNTERSCROLLINGTEXTEDIT_H
