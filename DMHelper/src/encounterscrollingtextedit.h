#ifndef ENCOUNTERSCROLLINGTEXTEDIT_H
#define ENCOUNTERSCROLLINGTEXTEDIT_H

#include "campaignobjectframe.h"
#include <QElapsedTimer>
#include "videoplayer.h"

namespace Ui {
class EncounterScrollingTextEdit;
}

class EncounterScrollingText;
class QGraphicsScene;
class QGraphicsTextItem;

class EncounterScrollingTextEdit : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit EncounterScrollingTextEdit(QWidget *parent = nullptr);
    virtual ~EncounterScrollingTextEdit() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    virtual bool isAnimated() override;

    EncounterScrollingText* getScrollingText() const;
    void setScrollingText(EncounterScrollingText* scrollingText);
    void unsetScrollingText(EncounterScrollingText* scrollingText);

public slots:
    void setScrollSpeed(double scrollSpeed);
    void setImageFile(const QString& imgFile);
    void browseImageFile();
    void setText(const QString& newText);
    void setFontFamily(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setFontBold(bool fontBold);
    void setFontItalics(bool fontItalics);
    void setAlignment(Qt::Alignment alignment);
    void setImageWidth(int imageWidth);
    void setColor(QColor color);

    void targetResized(const QSize& newSize);

    void runAnimation(bool animate);
    void stopAnimation();
    void rotatePublish(int rotation);

signals:
    void textChanged(const QString& text);
    void scrollSpeedChanged(double scrollSpeed);
    void imageFileChanged(const QString& imgFile);
//    void textChanged(const QString& newText);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void imageWidthChanged(int imageWidth);
    void colorChanged(QColor color);

    void animationStarted();
    void animateImage(QImage img);
    void showPublishWindow();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    void createVideoPlayer(bool dmPlayer);
    void cleanupPlayer();

private slots:
    /*
    void setPlainText();
    void setFontSize();
    */
    void setTextFont();
    //void setTextWidth();
    void setTextAlignment();
    void setTextColor();

    void startPublishTimer();
    void stopPublishTimer();

    void prepareImages();
    void prepareTextImage();
    void loadImage();
    void updateVideoBackground();

    void moveCursorToEnd();

private:

    Qt::Alignment getAlignment();
    QSize getRotatedTargetSize();

    Ui::EncounterScrollingTextEdit *ui;

    EncounterScrollingText* _scrollingText;
    int _backgroundWidth;
    QImage _backgroundImg;
    QImage _backgroundImgScaled;
    QImage _prescaledImg;
    QImage _textImg;
    QPointF _textPos;
    QSize _targetSize;
    QElapsedTimer _elapsed;
    int _timerId;

    VideoPlayer* _videoPlayer;
    bool _isDMPlayer;
    QImage _backgroundVideo;

    bool _animationRunning;
    int _rotation;
};

#endif // ENCOUNTERSCROLLINGTEXTEDIT_H
