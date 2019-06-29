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
    Encounter(encounterName, parent),
    _text()
{
}

EncounterText::EncounterText(const EncounterText& obj) :
    Encounter(obj),
    _text(obj._text)
{
}

void EncounterText::inputXML(const QDomElement &element, bool isImport)
{
    Encounter::inputXML(element, isImport);

    if( ( !element.firstChild().isNull() ) && ( element.firstChild().isCDATASection() ) )
    {
        QDomCDATASection cdata = element.firstChild().toCDATASection();
        setText(cdata.data());
    }
}

void EncounterText::widgetActivated(QWidget* widget)
{
    EncounterTextEdit* textEdit = dynamic_cast<EncounterTextEdit*>(widget);
    if(!textEdit)
        return;

    if(textEdit->getEncounter() != nullptr)
    {
        qDebug() << "[EncounterText] ERROR: Encounter not deactivated: " << textEdit->getEncounter()->getID() << " """ << textEdit->getEncounter()->getName();
        qDebug() << "[EncounterText] ERROR: Previous encounter will now be deactivated. This should happen previously!";
        textEdit->getEncounter()->widgetDeactivated(widget);
    }

    qDebug() << "[EncounterText] Widget Activated " << getID() << " """ << _name;
    textEdit->setEncounter(this);

    QList<QString> keys = textEdit->keys();
    QString text = getText();
    QTextDocument doc;
    doc.setHtml(text);

    QTextCursor cursor;
    for(int i = 0; i < keys.count(); ++i)
    {
        QString currentKey = keys.at(i);
        cursor = doc.find(currentKey);
        while(!cursor.isNull())
        {
            QTextCharFormat fmt = cursor.charFormat();
            if(!fmt.isAnchor())
            {
                fmt.setAnchor(true);
                fmt.setAnchorHref(QString("DMHelper@") + currentKey);
                cursor.setCharFormat(fmt);
            }

            cursor = doc.find(currentKey, cursor);
        }
    }

    textEdit->setHtml(doc.toHtml());
    _widget = widget;
    connect(textEdit, SIGNAL(textChanged()),this,SLOT(widgetChanged()));
}

void EncounterText::widgetDeactivated(QWidget* widget)
{
    EncounterTextEdit* textEdit = dynamic_cast<EncounterTextEdit*>(widget);
    if(!textEdit)
        return;

    qDebug() << "[EncounterText] Widget Deactivated " << getID() << " """ << _name;
    textEdit->setEncounter(nullptr);

    _text = textEdit->toHtml();
    disconnect(textEdit, nullptr, this, nullptr);
    _widget = nullptr;
}

int EncounterText::getType() const
{
    return DMHelper::EncounterType_Text;
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

    QTextDocument doc;
    doc.setHtml(newText);
    // qDebug() << "[EncounterText] " << getID() << " """ << _name << """ text set to: " << doc.toPlainText();

    if(_text != newText)
    {
        _text = newText;
        emit dirty();
    }
}

void EncounterText::widgetChanged()
{
    if(!_widget)
        return;

    //  qDebug() << "[EncounterText] Widget Changed " << getID() << " """ << _name;

    EncounterTextEdit* textEdit = dynamic_cast<EncounterTextEdit*>(_widget);
    if(!textEdit)
        return;

    setText(textEdit->toHtml());
}

void EncounterText::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    Q_UNUSED(targetDirectory);
    Q_UNUSED(isExport);

    QDomCDATASection cdata = doc.createCDATASection(getText());
    element.appendChild(cdata);
}
