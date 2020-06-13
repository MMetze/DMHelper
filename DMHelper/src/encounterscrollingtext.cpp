#include "encounterscrollingtext.h"
#include "encounterscrollingtextedit.h"
#include "dmconstants.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDir>
#include <QDomCDATASection>
#include <QGuiApplication>
#include <QDebug>

EncounterScrollingText::EncounterScrollingText(const QString& encounterName, QObject *parent) :
    EncounterText(encounterName, parent),
    _scrollSpeed(25),
    _imgFile(),
    _fontFamily(QGuiApplication::font().family()),
    _fontSize(12),
    _fontBold(false),
    _fontItalics(false),
    _alignment(Qt::AlignCenter),
    _imageWidth(80),
    _fontColor(Qt::white)
{
}

/*
EncounterScrollingText::EncounterScrollingText(const EncounterScrollingText& obj) :
    Encounter(obj),
    _scrollSpeed(obj._scrollSpeed),
    _imgFile(obj._imgFile),
    _text(obj._text),
    _fontFamily(obj._fontFamily),
    _fontSize(obj._fontSize),
    _fontBold(obj._fontBold),
    _fontItalics(obj._fontItalics),
    _alignment(obj._alignment),
    _imageWidth(obj._imageWidth),
    _fontColor(obj._fontColor)
{
}
*/

void EncounterScrollingText::inputXML(const QDomElement &element, bool isImport)
{
    int scrollSpeed = element.attribute("scrollSpeed").toInt();
    setScrollSpeed(scrollSpeed > 0 ? scrollSpeed : 25);
    setImageFile(element.attribute("imageFile"));
    setFontFamily(element.attribute("fontFamily"));
    setFontSize(element.attribute("fontSize").toInt());
    setFontBold(element.attribute("fontBold",QString::number(0)).toInt());
    setFontItalics(element.attribute("fontItalics",QString::number(0)).toInt());
    setAlignment(static_cast<Qt::Alignment>(element.attribute("alignment", "4").toInt()));
    setImageWidth(element.attribute("imageWidth", "80").toInt());

    QString colorName = element.attribute("fontColor");
    if(QColor::isValidColor(colorName))
    {
        QColor newColor(colorName);
        setFontColor(newColor);
    }

    EncounterText::inputXML(element, isImport);
}

/*
void EncounterScrollingText::widgetActivated(QWidget* widget)
{
    EncounterScrollingTextEdit* textEdit = dynamic_cast<EncounterScrollingTextEdit*>(widget);
    if(!textEdit)
        return;

    if(textEdit->getScrollingText() != nullptr)
    {
        qDebug() << "[EncounterScrollingText] ERROR: Scrolling text not deactivated: " << textEdit->getScrollingText()->getID() << " """ << textEdit->getScrollingText()->getName();
        qDebug() << "[EncounterScrollingText] ERROR: Previous scrolling text will now be deactivated. This should happen previously!";
        textEdit->getScrollingText()->widgetDeactivated(widget);
    }

    textEdit->setScrollingText(this);

    _widget = widget;
    connect(textEdit, SIGNAL(scrollSpeedChanged(double)), this, SLOT(setScrollSpeed(double)));
    connect(textEdit, SIGNAL(imgFileChanged(const QString&)), this, SLOT(setImageFile(QString)));
    connect(textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(setText(QString)));
    connect(textEdit, SIGNAL(fontFamilyChanged(const QString&)), this, SLOT(setFontFamily(QString)));
    connect(textEdit, SIGNAL(fontSizeChanged(int)), this, SLOT(setFontSize(int)));
    connect(textEdit, SIGNAL(fontBoldChanged(bool)), this, SLOT(setFontBold(bool)));
    connect(textEdit, SIGNAL(fontItalicsChanged(bool)), this, SLOT(setFontItalics(bool)));
    connect(textEdit, SIGNAL(alignmentChanged(int)), this, SLOT(setAlignment(int)));
    connect(textEdit, SIGNAL(imageWidthChanged(int)), this, SLOT(setImageWidth(int)));
    connect(textEdit, SIGNAL(colorChanged(QColor)), this, SLOT(setFontColor(QColor)));
}

void EncounterScrollingText::widgetDeactivated(QWidget* widget)
{
    EncounterScrollingTextEdit* textEdit = dynamic_cast<EncounterScrollingTextEdit*>(widget);
    if(!textEdit)
        return;

    textEdit->unsetScrollingText(this);

    disconnect(textEdit, nullptr, this, nullptr);
    _widget = nullptr;
}
*/

int EncounterScrollingText::getObjectType() const
{
    return DMHelper::CampaignType_ScrollingText;
}

int EncounterScrollingText::getScrollSpeed() const
{
    return _scrollSpeed;
}

QString EncounterScrollingText::getImageFile() const
{
    return _imgFile;
}

QString EncounterScrollingText::getFontFamily() const
{
    return _fontFamily;
}

int EncounterScrollingText::getFontSize() const
{
    return _fontSize;
}

bool EncounterScrollingText::getFontBold() const
{
    return _fontBold;
}

bool EncounterScrollingText::getFontItalics() const
{
    return _fontItalics;
}

Qt::Alignment EncounterScrollingText::getAlignment() const
{
    return _alignment;
}

int EncounterScrollingText::getImageWidth() const
{
    return _imageWidth;
}

QColor EncounterScrollingText::getFontColor() const
{
    return _fontColor;
}

void EncounterScrollingText::setScrollSpeed(int scrollSpeed)
{
    if(_scrollSpeed != scrollSpeed)
    {
        _scrollSpeed = scrollSpeed;
        emit scrollSpeedChanged(_scrollSpeed);
        emit dirty();
    }
}

void EncounterScrollingText::setImageFile(const QString& imgFile)
{
    if(_imgFile != imgFile)
    {
        _imgFile = imgFile;
        emit imageFileChanged(_imgFile);
        emit dirty();
    }
}

void EncounterScrollingText::setFontFamily(const QString& fontFamily)
{
    if(_fontFamily != fontFamily)
    {
        _fontFamily = fontFamily;
        emit fontFamilyChanged(_fontFamily);
        emit dirty();
    }
}

void EncounterScrollingText::setFontSize(int fontSize)
{
    if(_fontSize != fontSize)
    {
        _fontSize = fontSize;
        emit fontSizeChanged(_fontSize);
        emit dirty();
    }
}

void EncounterScrollingText::setFontBold(bool fontBold)
{
    if(_fontBold != fontBold)
    {
        _fontBold = fontBold;
        emit fontBoldChanged(_fontBold);
        emit dirty();
    }
}

void EncounterScrollingText::setFontItalics(bool fontItalics)
{
    if(_fontItalics != fontItalics)
    {
        _fontItalics = fontItalics;
        emit fontItalicsChanged(_fontItalics);
        emit dirty();
    }
}

void EncounterScrollingText::setAlignment(Qt::Alignment alignment)
{
    if(_alignment != alignment)
    {
        _alignment = alignment;
        emit alignmentChanged(_alignment);
        emit dirty();
    }
}

void EncounterScrollingText::setImageWidth(int imageWidth)
{
    if((_imageWidth != imageWidth) && (_imageWidth >= 10) && (_imageWidth <= 100))
    {
        _imageWidth = imageWidth;
        emit imageWidthChanged(_imageWidth);
        emit dirty();
    }
}

void EncounterScrollingText::setFontColor(QColor fontColor)
{
    if(_fontColor != fontColor)
    {
        _fontColor = fontColor;
        emit fontColorChanged(_fontColor);
        emit dirty();
    }
}

/*
void EncounterScrollingText::widgetChanged()
{
    if(!_widget)
        return;

    EncounterScrollingTextEdit* textEdit = dynamic_cast<EncounterScrollingTextEdit*>(_widget);
    if(!textEdit)
        return;
}
*/

QDomElement EncounterScrollingText::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("scrolling-object");
}

void EncounterScrollingText::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    EncounterText::internalOutputXML(doc, element, targetDirectory, isExport);

    element.setAttribute( "scrollSpeed", getScrollSpeed());
    element.setAttribute( "imageFile", targetDirectory.relativeFilePath(getImageFile()) );
    element.setAttribute( "fontFamily", getFontFamily());
    element.setAttribute( "fontSize", getFontSize());
    element.setAttribute( "fontBold", static_cast<int>(getFontBold()));
    element.setAttribute( "fontItalics", static_cast<int>(getFontItalics()));
    element.setAttribute( "alignment", static_cast<int>(getAlignment()));
    element.setAttribute( "imageWidth", getImageWidth());
    element.setAttribute( "fontColor", getFontColor().name());
}
