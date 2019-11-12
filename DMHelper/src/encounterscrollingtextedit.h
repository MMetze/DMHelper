#ifndef ENCOUNTERSCROLLINGTEXTEDIT_H
#define ENCOUNTERSCROLLINGTEXTEDIT_H

#include <QFrame>
#include <QElapsedTimer>
#include "videoplayer.h"

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
    virtual void timerEvent(QTimerEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    void createVideoPlayer(bool dmPlayer);
    void cleanupPlayer();

private slots:
    void setPlainText();
    void setFontSize();
    void setAlignment();
    void setColor();
    void setTextFont();
    void setTextWidth();
    void browseImageFile();

    void runAnimation();
    void rotatePublish();
    void startPublishTimer();
    void stopPublishTimer();

    void prepareImages();
    void prepareTextImage();
    void loadImage();
    void updateVideoBackground();

private:

    Qt::AlignmentFlag getAlignment();
    QSize getRotatedTargetSize();

    Ui::EncounterScrollingTextEdit *ui;

    EncounterScrollingText* _scrollingText;
    int _backgroundWidth;
    QImage _backgroundImg;
    QImage _prescaledImg;
    QImage _textImg;
    QPointF _textPos;
    QSize _targetSize;
    QElapsedTimer _elapsed;
    int _timerId;

    VideoPlayer* _videoPlayer;
    bool _isDMPlayer;
    QImage _backgroundVideo;
};

#endif // ENCOUNTERSCROLLINGTEXTEDIT_H
