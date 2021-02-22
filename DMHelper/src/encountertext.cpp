#include "encountertext.h"
#include "dmconstants.h"
#include "encountertextedit.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>
#include <QTextDocument>
#include <QTextCursor>
#include <QDir>
#include <QDebug>

const int ENCOUNTERTYPE_SCROLLINGTEXT = 5;

EncounterText::EncounterText(const QString& encounterName, QObject *parent) :
    CampaignObjectBase(encounterName, parent),
    _text(),
    _imageFile(),
    _textWidth(80),
    _animated(false),
    _scrollSpeed(25)
{
}

void EncounterText::inputXML(const QDomElement &element, bool isImport)
{
    extractTextNode(element, isImport);

    setImageFile(element.attribute("imageFile"));
    setTextWidth(element.attribute("textWidth", "80").toInt());
    int scrollSpeed = element.attribute("scrollSpeed").toInt();
    setScrollSpeed(scrollSpeed > 0 ? scrollSpeed : 25);
    setAnimated(static_cast<bool>(element.attribute("animated", QString::number(0)).toInt()));

    int encounterType;
    bool ok = false;
    if(element.tagName() == QString("encounter"))
        encounterType = element.attribute("type").toInt(&ok);

    if((element.tagName() == QString("scrolling-object")) ||
       (element.tagName() == QString("encounter") && (ok) && (encounterType == ENCOUNTERTYPE_SCROLLINGTEXT)))
    {
        // Compatibility mode
        QTextDocument document(_text);
        QTextCursor cursor(&document);

        cursor.select(QTextCursor::Document);

        QString fontFamily = element.attribute("fontFamily");
        QString fontSize = element.attribute("fontSize");
        QString fontBold = element.attribute("fontBold",QString::number(0));
        QString fontItalics = element.attribute("fontItalics",QString::number(0));

        QTextCharFormat format = cursor.charFormat();
        QFont formatFont(fontFamily,
                         fontSize.toInt(),
                         fontBold.toInt(),
                         static_cast<bool>(fontItalics.toInt()));
        format.setFont(formatFont);
        QString colorName = element.attribute("fontColor");
        if(QColor::isValidColor(colorName))
            format.setForeground(QBrush(QColor(colorName)));
        cursor.mergeCharFormat(format);

        QTextBlockFormat blockFormat = cursor.blockFormat();
        blockFormat.setAlignment(static_cast<Qt::Alignment>(element.attribute("alignment", "4").toInt()));
        cursor.mergeBlockFormat(blockFormat);

        cursor.clearSelection();
        setText(document.toHtml());

        setTextWidth(element.attribute("imageWidth", "80").toInt());
        setAnimated(true);
    }

    CampaignObjectBase::inputXML(element, isImport);
}

int EncounterText::getObjectType() const
{
    return DMHelper::CampaignType_Text;
}

QString EncounterText::getText() const
{
    return _text;
}

QString EncounterText::getImageFile() const
{
    return _imageFile;
}

bool EncounterText::getAnimated() const
{
    return _animated;
}

int EncounterText::getScrollSpeed() const
{
    return _scrollSpeed;
}

int EncounterText::getTextWidth() const
{
    return _textWidth;
}

void EncounterText::setText(const QString& newText)
{
    if(newText.isEmpty())
    {
        qDebug() << "[EncounterText] ERROR: Attempting to set encounter text to nothing! This is assumed to be an error and needs to be further investigated!";
    }

    if(_text != newText)
    {
        _text = newText;
        /*
        QTextDocument doc;
        doc.setHtml(newText);
        */

        emit textChanged(_text);
        emit dirty();
    }
}

void EncounterText::setImageFile(const QString& imageFile)
{
    if(_imageFile == imageFile)
        return;

    _imageFile = imageFile;

    emit imageFileChanged(_imageFile);
    emit dirty();
}

void EncounterText::setAnimated(bool animated)
{
    if(_animated == animated)
        return;

    _animated = animated;

    emit animatedChanged(_animated);
    emit dirty();
}

void EncounterText::setScrollSpeed(int scrollSpeed)
{
    if(_scrollSpeed == scrollSpeed)
        return;

    _scrollSpeed = scrollSpeed;

    emit scrollSpeedChanged(_scrollSpeed);
    emit dirty();
}

void EncounterText::setTextWidth(int textWidth)
{
    if(_textWidth == textWidth)
        return;

    _textWidth = textWidth;

    emit textWidthChanged(_textWidth);
    emit dirty();
}

QDomElement EncounterText::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("entry-object");
}

void EncounterText::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);

    element.setAttribute("imageFile", targetDirectory.relativeFilePath(getImageFile()));
    element.setAttribute("textWidth", getTextWidth());
    element.setAttribute("scrollSpeed", getScrollSpeed());
    element.setAttribute("animated", getAnimated());

    QDomCDATASection cdata = doc.createCDATASection(getText());
    element.appendChild(cdata);
}

bool EncounterText::belongsToObject(QDomElement& element)
{
    if((getName() == "Notes") &&
       (element.tagName() == "encounter") &&
       (element.attribute(QString("name")) == QString("")))
    {
        // DEPRECATED v2.0
        // This is compatibility mode only to avoid an "unknown" node when importing an old-style campaign
        return true;
    }
    else
    {
        return CampaignObjectBase::belongsToObject(element);
    }
}

void EncounterText::internalPostProcessXML(const QDomElement &element, bool isImport)
{
    // DEPRECATED v2.0
    // This is compatibility mode only to avoid an "unknown" node when importing an old-style campaign
    if(getName() != QString("Notes"))
        return;

    CampaignObjectBase* parentObject = qobject_cast<CampaignObjectBase*>(parent());
    if((!parentObject) || (parentObject->getObjectType() != DMHelper::CampaignType_Campaign))
        return;

    QDomElement childElement = element.firstChildElement("encounter");
    if((childElement.isNull()) || (childElement.attribute("name") != QString("")))
        return;

    // Grab the text from the child node for this node
    extractTextNode(childElement, isImport);

    CampaignObjectBase::internalPostProcessXML(element, isImport);
}

void EncounterText::extractTextNode(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    QDomNode childNode = element.firstChild();
    while(!childNode.isNull())
    {
        if(childNode.isCDATASection())
        {
            QDomCDATASection cdata = childNode.toCDATASection();
            setText(cdata.data());
            return;
        }
        childNode = childNode.nextSibling();
    }
}
