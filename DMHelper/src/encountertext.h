#ifndef ENCOUNTERTEXT_H
#define ENCOUNTERTEXT_H

#include "campaignobjectbase.h"
#include "layerscene.h"
#include <QDomElement>

class QDomDocument;

class EncounterText : public CampaignObjectBase
{
    Q_OBJECT
public:

    explicit EncounterText(const QString& encounterName = QString(), QObject *parent = nullptr);
    virtual ~EncounterText() override;

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;
    virtual QIcon getDefaultIcon() override;

    // For support of GlobalSearch_Interface
    virtual bool matchSearch(const QString& searchString) const override;

    // Text
    virtual QString getText() const;
    virtual QString getImageFile() const;

    // Animation
    virtual bool getAnimated() const;
    virtual int getScrollSpeed() const;
    virtual int getTextWidth() const;

    // Translation
    virtual bool getTranslated() const;
    virtual QString getTranslatedText() const;

    bool isInitialized() const;
    LayerScene& getLayerScene();
    const LayerScene& getLayerScene() const;

public slots:
    bool initialize();
    void uninitialize();

    // Text
    virtual void setText(const QString& newText);
    virtual void setImageFile(const QString& imageFile);

    // Animation
    virtual void setAnimated(bool animated);
    virtual void setScrollSpeed(int scrollSpeed);
    virtual void setTextWidth(int textWidth);

    // Translation
    virtual void setTranslated(bool translated);
    virtual void setTranslatedText(const QString& translatedText);

signals:
    // Text
    void textChanged(const QString& text);
//    void imageFileChanged(const QString& imageFile);

    // Animation
    void animatedChanged(bool animated);
    void scrollSpeedChanged(int scrollSpeed);
    void textWidthChanged(int textWidth);

    // Translation
    void translatedChanged(bool translated);
    void translatedTextChanged(const QString& translatedText);

protected slots:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

protected:
    virtual void createTextNode(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);
    virtual void extractTextNode(const QDomElement &element, bool isImport);

    // Text
    LayerScene _layerScene;
    QString _text;
    QString _translatedText;
    QString _imageFile; // For compatibility only
    int _textWidth;
    bool _initialized;

    // Animation
    bool _animated;
    bool _translated;
    int _scrollSpeed;
};

#endif // ENCOUNTERTEXT_H
