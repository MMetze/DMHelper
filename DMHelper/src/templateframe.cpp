#include "templateframe.h"
#include "templateobject.h"
#include "templatefactory.h"
#include <QBoxLayout>
#include <QScrollArea>
#include <QMessageBox>
#include <QTextEdit>
#include <QKeyEvent>
#include <QDebug>

TemplateFrame::TemplateFrame() :
    _uiFilename()
{
}

TemplateFrame::~TemplateFrame()
{
}

void TemplateFrame::handleEditBoxChange(QWidget* editWidget, TemplateObject* templateObject, const QString& value)
{
    if((!templateObject) || (!editWidget))
        return;

    int widgetIndex = 0;
    QBoxLayout* scrollLayout = nullptr;
    QString scrollKey;

    QWidget* parentWidget = editWidget->parentWidget();

    // Traverse up the widget hierarchy until the QScrollArea is found
    while(parentWidget)
    {
        if(QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(parentWidget))
        {
            if(scrollArea->widget())
            {
                scrollLayout = dynamic_cast<QBoxLayout*>(scrollArea->widget()->layout());
                scrollKey = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
                break;
            }
        }
        parentWidget = parentWidget->parentWidget();
    }

    if(scrollLayout)
        widgetIndex = scrollLayout->indexOf(editWidget->parentWidget());

    QString editWidgetKey = editWidget->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
    if((!scrollKey.isEmpty()) && (widgetIndex >= 0))
        templateObject->setListValue(scrollKey, widgetIndex, editWidgetKey, value);
    else
        templateObject->setValue(editWidgetKey, value);
}

void TemplateFrame::handleAddResource(QWidget* widget, TemplateObject* templateObject)
{
    if((!widget) || (!widget->parentWidget()) || (!templateObject) || (!templateObject->getFactory()))
        return;

    int widgetIndex = 0;
    QBoxLayout* scrollLayout = nullptr;
    QScrollArea* scrollArea = dynamic_cast<QScrollArea*>(widget);

    if(scrollArea)
    {
        if(scrollArea->widget())
            scrollLayout = dynamic_cast<QBoxLayout*>(scrollArea->widget()->layout());
    }
    else
    {
        scrollLayout = dynamic_cast<QBoxLayout*>(widget->parentWidget()->layout());
        if(!scrollLayout)
            return;

        widgetIndex = scrollLayout->indexOf(widget);
        if(widgetIndex < 0)
            return;

        // First parent is the dynamically created frame withing the scroll area
        if((widget->parentWidget()->parentWidget()) && (widget->parentWidget()->parentWidget()->parentWidget()))
            scrollArea = dynamic_cast<QScrollArea*>(widget->parentWidget()->parentWidget()->parentWidget());
    }

    if((!scrollLayout) || (widgetIndex < 0) || (!scrollArea))
        return;

    QString keyString = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
    QString widgetString = scrollArea->property(TemplateFactory::TEMPLATE_WIDGET).toString();

    QWidget* newWidget = templateObject->getFactory()->createResourceWidget(keyString, widgetString, getUIFilename());
    if(!newWidget)
    {
        qDebug() << "[TemplateFrame] ERROR: Unable to create the widget: " << widgetString << ", for scroll area: " << keyString;
        return;
    }

    QHash<QString, QVariant> newHash = templateObject->createListEntry(keyString, widgetIndex);
    QHash<QString, DMHAttribute> hashAttributes = templateObject->getFactory()->getElementList(keyString);

    // Walk through the loaded UI Widget and allocate the appropriate values to the UI elements
    templateObject->getFactory()->populateWidget(newWidget, templateObject, this, &newHash, &hashAttributes, widgetIndex, keyString);

    newWidget->installEventFilter(getFrameObject());
    scrollLayout->insertWidget(widgetIndex, newWidget);
}

void TemplateFrame::handleRemoveResource(QWidget* widget, TemplateObject* templateObject)
{
    if((!widget) || (!widget->parentWidget()) || (!templateObject) || (!templateObject->getFactory()))
        return;

    QLayout* parentLayout = dynamic_cast<QLayout*>(widget->parentWidget()->layout());
    if(!parentLayout)
        return;

    int widgetIndex = parentLayout->indexOf(widget);
    if(widgetIndex < 0)
        return;

    // First parent is the dynamically created frame withing the scroll area
    QScrollArea* scrollArea = nullptr;
    if((widget->parentWidget()->parentWidget()) && (widget->parentWidget()->parentWidget()->parentWidget()))
        scrollArea = dynamic_cast<QScrollArea*>(widget->parentWidget()->parentWidget()->parentWidget());
    if(scrollArea == nullptr)
        return;

    if(QMessageBox::question(nullptr, QObject::tr("Remove Resource"), QObject::tr("Are you sure you want to remove this resource?")) != QMessageBox::Yes)
        return;

    QString keyString = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
    QList<QVariant> listValue = templateObject->getListValue(keyString);
    if(listValue.isEmpty())
        return;

    listValue.remove(widgetIndex);
    templateObject->setValue(keyString, listValue);

    parentLayout->removeWidget(widget);
    widget->deleteLater();
}

QString TemplateFrame::getUIFilename() const
{
    return _uiFilename;
}

void TemplateFrame::postLoadConfiguration(QWidget* owner, QWidget* uiWidget)
{
    if((!owner) || (!uiWidget))
        return;

    // Install this dialog as the event filter for each text edit to handle formatting
    QList<QTextEdit*> textEdits = uiWidget->findChildren<QTextEdit*>();
    for(QTextEdit* textEdit : textEdits)
    {
        if(textEdit)
            textEdit->installEventFilter(owner);
    }
}

bool TemplateFrame::localEventFilter(QObject* object, QEvent* event)
{
    if((!object) || (!event))
        return false;

    // Check if the object is a text edit and handle basic text formatting for it
    if(event->type() != QEvent::KeyPress)
        return false;

    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if(!keyEvent)
        return false;

    QTextEdit* watched = qobject_cast<QTextEdit*>(object);
    if(!watched)
        return false;

    if((keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
    {
        QTextCharFormat currentFormat = watched->currentCharFormat();
        QTextCharFormat deltaFormat;

        if(keyEvent->key() == Qt::Key_B)
        {
            deltaFormat.setFontWeight(currentFormat.fontWeight() == QFont::Normal ? QFont::Bold : QFont::Normal);
            watched->mergeCurrentCharFormat(deltaFormat);
        }
        else if(keyEvent->key() == Qt::Key_I)
        {
            deltaFormat.setFontItalic(!currentFormat.fontItalic());
            watched->mergeCurrentCharFormat(deltaFormat);
        }
        else if(keyEvent->key() == Qt::Key_U)
        {
            deltaFormat.setFontUnderline(!currentFormat.fontUnderline());
            watched->mergeCurrentCharFormat(deltaFormat);
        }
    }

    return false;
}
