#ifndef ENCOUNTERTEXTEDIT_H
#define ENCOUNTERTEXTEDIT_H

#include "campaignobjectframe.h"
#include "texteditformatterframe.h"
#include "videoplayer.h"
#include <QElapsedTimer>

namespace Ui {
class EncounterTextEdit;
}

class EncounterText;

class EncounterTextEdit : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit EncounterTextEdit(QWidget *parent = nullptr);
    virtual ~EncounterTextEdit() override;

    virtual void activateObject(CampaignObjectBase* object) override;
    virtual void deactivateObject() override;

    void setKeys(const QList<QString>& keys);
    QList<QString> keys();

    EncounterText* getEncounter() const;
    void setEncounter(EncounterText* encounter);
    void unsetEncounter(EncounterText* encounter);

    QString toHtml() const;
    QString toPlainText() const;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void clear();
    void setHtml();

    void setBackgroundImage(bool on);
    void setImageFile(const QString& imageFile);
    void browseImageFile();

    void setFont(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setBold(bool bold);
    void setItalics(bool italics);
    void setUnderline(bool underline);
    void setColor(QColor color);
    void setAlignment(Qt::Alignment alignment);

    void hyperlinkClicked();
    void setTextWidth(int textWidth);

    void setAnimated(bool animated);
    void setScrollSpeed(int scrollSpeed);
    void stopAnimation();
    void rewind();

    void setTranslated(bool translated);

    void targetResized(const QSize& newSize);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;

signals:
    void anchorClicked(const QUrl &link);

    void imageFileChanged(const QString&);

    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void fontUnderlineChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void colorChanged(QColor color);

    void setHyperlinkActive(bool active);
    void textWidthChanged(int textWidth);

    void animatedChanged(bool animated);
    void scrollSpeedChanged(int scrollSpeed);
    void translatedChanged(bool translated);

    void publishImage(QImage image);
    void animationStarted(CampaignObjectBase* animatedObject);
    void animateImage(QImage image);
    void animationStopped();
    void showPublishWindow();

protected slots:
    void updateAnchors();
    void storeEncounter();
    void readEncounter();

    void takeFocus();
    void loadImage();
    void updateVideoBackground();

    void startPublishTimer();
    void stopPublishTimer();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    void scaleBackgroundImage();
    void prepareImages();
    void prepareTextImage();
    void drawTextImage(QPaintDevice* target);

    void createVideoPlayer(bool dmPlayer);
    void cleanupPlayer();

    bool isVideo() const;
    bool isAnimated() const;
    void setPublishCheckable();
    QSize getRotatedTargetSize();

    Ui::EncounterTextEdit *ui;

    QList<QString> _keys;
    EncounterText* _encounter;
    TextEditFormatterFrame* _formatter;

    QImage _backgroundImage;
    QImage _backgroundImageScaled;
    QImage _prescaledImage;
    QImage _textImage;

    VideoPlayer* _videoPlayer;
    bool _isDMPlayer;
    QImage _backgroundVideo;

    QSize _targetSize;
    int _rotation;

    bool _animationRunning;
    QPointF _textPos;
    QElapsedTimer _elapsed;
    int _timerId;
};

#endif // ENCOUNTERTEXTEDIT_H
