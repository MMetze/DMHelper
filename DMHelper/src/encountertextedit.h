#ifndef ENCOUNTERTEXTEDIT_H
#define ENCOUNTERTEXTEDIT_H

#include "campaignobjectframe.h"
#include "texteditformatterframe.h"
#include "videoplayer.h"
#include "layer.h"
#include <QElapsedTimer>

namespace Ui {
class EncounterTextEdit;
}

class EncounterText;
class PublishGLTextRenderer;
class PublishGLRenderer;

class EncounterTextEdit : public CampaignObjectFrame
{
    Q_OBJECT

public:
    explicit EncounterTextEdit(QWidget *parent = nullptr);
    virtual ~EncounterTextEdit() override;

    virtual void activateObject(CampaignObjectBase* object, PublishGLRenderer* currentRenderer) override;
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

    void setFont(const QString& fontFamily);
    void setFontSize(int fontSize);
    void setBold(bool bold);
    void setItalics(bool italics);
    void setUnderline(bool underline);
    void setColor(const QColor& color);
    void setAlignment(Qt::Alignment alignment);
    void setPasteRich(bool pasteRich);

    void hyperlinkClicked();
    void setTextWidth(int textWidth);

    void setAnimated(bool animated);
    void setScrollSpeed(int scrollSpeed);
    void rewind();
    void playPause(bool play);

    void setTranslated(bool translated);
    void setCodeView(bool active);

    void layerSelected(int selected);

    // Publish slots from CampaignObjectFrame
    virtual void publishClicked(bool checked) override;
    virtual void setRotation(int rotation) override;
    virtual void setBackgroundColor(const QColor& color) override;
    virtual void editLayers() override;

signals:
    void anchorClicked(const QUrl &link);

    void imageFileChanged(const QString&);

    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void fontUnderlineChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void colorChanged(const QColor& color);

    void playPauseChanged(bool playing);

    void setHyperlinkActive(bool active);
    void textWidthChanged(int textWidth);

    void animatedChanged(bool animated);
    void scrollSpeedChanged(int scrollSpeed);
    void translatedChanged(bool translated);
    void codeViewChanged(bool active);
    void codeViewVisible(bool visible);

    void registerRenderer(PublishGLRenderer* renderer);
    void setLayers(QList<Layer*> layers, int selected);

    void publishImage(QImage image);
    void showPublishWindow();

protected slots:
    void updateAnchors();
    void storeEncounter();
    void readEncounter();
    void updateEncounter();

    void takeFocus();
    void loadImage();
    void handleLayersChanged();

    void refreshImage();

    void triggerEncounterChanged();
    void triggerUpdateAnchor();

    void sceneRectUpdated(const QSize& size);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

    void scaleBackgroundImage();
    void prepareImages();
    void prepareTextImage();
    QImage getDocumentTextImage(int renderWidth);
    void drawTextImage(QPaintDevice* target);

    void setPublishCheckable();
    QSize getRotatedTargetSize();
    int getRotatedTargetWidth();

    void cancelTimers();

    Ui::EncounterTextEdit *ui;

    QList<QString> _keys;
    EncounterText* _encounter;
    PublishGLTextRenderer* _renderer;
    TextEditFormatterFrame* _formatter;

    QImage _backgroundImage;
    QImage _backgroundImageScaled;
    QImage _prescaledImage;
    QImage _textImage;

    bool _isDMPlayer;
    bool _isPublishing;
    bool _isCodeView;

    QSize _targetSize;
    int _rotation;

    QPointF _textPos;

    int _encounterChangedTimer;
    int _updateAnchorTimer;
};

#endif // ENCOUNTERTEXTEDIT_H
