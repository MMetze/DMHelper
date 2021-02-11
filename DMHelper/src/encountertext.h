#ifndef ENCOUNTERTEXT_H
#define ENCOUNTERTEXT_H

#include "campaignobjectbase.h"
#include <QDomElement>

class QDomDocument;

class EncounterText : public CampaignObjectBase
{
    Q_OBJECT
public:

    explicit EncounterText(const QString& encounterName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual int getObjectType() const override;

    // Text
    virtual QString getText() const;
    virtual QString getImageFile() const;

    // Animation
    virtual bool getAnimated() const;
    virtual int getScrollSpeed() const;

    //virtual QString getFontFamily() const;
    //virtual int getFontSize() const;
    //virtual bool getFontBold() const;
    //virtual bool getFontItalics() const;
    //virtual Qt::Alignment getAlignment() const;
    virtual int getTextWidth() const;
    //virtual QColor getFontColor() const;

public slots:
    //Text
    virtual void setText(const QString& newText);
    virtual void setImageFile(const QString& imageFile);

    // Animation
    virtual void setAnimated(bool animated);
    virtual void setScrollSpeed(int scrollSpeed);

    //virtual void setFontFamily(const QString& fontFamily);
    //virtual void setFontSize(int fontSize);
    //virtual void setFontBold(bool fontBold);
    //virtual void setFontItalics(bool fontItalics);
    //virtual void setAlignment(Qt::Alignment alignment);
    virtual void setTextWidth(int textWidth);
    //virtual void setFontColor(QColor fontColor);

signals:
    // Text
    void textChanged(const QString& text);
    void imageFileChanged(const QString& imageFile);

    // Animation
    void animatedChanged(bool animated);
    void scrollSpeedChanged(int scrollSpeed);
    //void fontFamilyChanged(const QString& fontFamily);
    //void fontSizeChanged(int fontSize);
    //void fontBoldChanged(bool fontBold);
    //void fontItalicsChanged(bool fontItalics);
    //void alignmentChanged(Qt::Alignment alignment);
    void textWidthChanged(int textWidth);
    //void fontColorChanged(QColor fontColor);

protected slots:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual bool belongsToObject(QDomElement& element) override;
    virtual void internalPostProcessXML(const QDomElement &element, bool isImport) override;

protected:
    void extractTextNode(const QDomElement &element, bool isImport);

    // Text
    QString _text;
    QString _imageFile;
    int _textWidth;

    // Animation
    bool _animated;
    int _scrollSpeed;
};

#endif // ENCOUNTERTEXT_H
