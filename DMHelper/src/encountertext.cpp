#include "encountertext.h"
#include "dmconstants.h"
#include "encountertextedit.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomCDATASection>
#include <QTextDocument>
#include <QTextCursor>
#include <QDebug>

EncounterText::EncounterText(const QString& encounterName, QObject *parent) :
    CampaignObjectBase(encounterName, parent),
    _text()
{
}

void EncounterText::inputXML(const QDomElement &element, bool isImport)
{
    extractTextNode(element, isImport);
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

void EncounterText::setText(const QString& newText)
{
    if(newText.isEmpty())
    {
        qDebug() << "[EncounterText] ERROR: Attempting to set encounter text to nothing! This is assumed to be an error and needs to be further investigated!";
    }

    if(_text != newText)
    {
        _text = newText;
        QTextDocument doc;
        doc.setHtml(newText);
        // qDebug() << "[EncounterText] " << getID() << " """ << _name << """ text set to: " << doc.toPlainText();

        emit textChanged(_text);
        emit dirty();
    }
}

QDomElement EncounterText::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("text-object");
}

void EncounterText::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    CampaignObjectBase::internalOutputXML(doc, element, targetDirectory, isExport);

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
