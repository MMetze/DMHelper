#ifndef ENCOUNTERSCROLLINGTEXT_H
#define ENCOUNTERSCROLLINGTEXT_H

#include "encountertext.h"
#include <QColor>
#include <QDomElement>

class QDomDocument;

class EncounterScrollingText : public EncounterText
{
    Q_OBJECT
public:

    explicit EncounterScrollingText(const QString& encounterName = QString(), QObject *parent = nullptr);
    //explicit EncounterScrollingText(const EncounterScrollingText& obj);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;

    // Base functions to handle UI widgets
    //virtual void widgetActivated(QWidget* widget) override;
    //virtual void widgetDeactivated(QWidget* widget) override;

    virtual int getObjectType() const override;

    virtual int getScrollSpeed() const;
    virtual QString getImageFile() const;
    virtual QString getFontFamily() const;
    virtual int getFontSize() const;
    virtual bool getFontBold() const;
    virtual bool getFontItalics() const;
    virtual Qt::Alignment getAlignment() const;
    virtual int getImageWidth() const;
    virtual QColor getFontColor() const;

public slots:
    virtual void setScrollSpeed(int scrollSpeed);
    virtual void setImageFile(const QString& imgFile);
    virtual void setFontFamily(const QString& fontFamily);
    virtual void setFontSize(int fontSize);
    virtual void setFontBold(bool fontBold);
    virtual void setFontItalics(bool fontItalics);
    virtual void setAlignment(Qt::Alignment alignment);
    virtual void setImageWidth(int imageWidth);
    virtual void setFontColor(const QColor& fontColor);

signals:
    void scrollSpeedChanged(int scrollSpeed);
    void imageFileChanged(const QString& imgFile);
    void fontFamilyChanged(const QString& fontFamily);
    void fontSizeChanged(int fontSize);
    void fontBoldChanged(bool fontBold);
    void fontItalicsChanged(bool fontItalics);
    void alignmentChanged(Qt::Alignment alignment);
    void imageWidthChanged(int imageWidth);
    void fontColorChanged(const QColor& fontColor);

protected slots:
    //virtual void widgetChanged() override;
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;

protected:

    int _scrollSpeed;
    QString _imgFile;
    QString _fontFamily;
    int _fontSize;
    bool _fontBold;
    bool _fontItalics;
    Qt::Alignment _alignment;
    int _imageWidth;
    QColor _fontColor;
};

#endif // ENCOUNTERSCROLLINGTEXT_H
