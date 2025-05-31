#include "templateresourcelayout.h"
#include <QWidget>
#include <QCheckBox>
#include <QMouseEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>

TemplateResourceLayout::TemplateResourceLayout(const QString& key, const ResourcePair& value) :
    QHBoxLayout(),
    _key(key),
    _listIndex(-1),
    _listKey(),
    _value(value)
{
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setContentsMargins(0, 0, 0, 0);

    createCheckboxes();
}

TemplateResourceLayout::TemplateResourceLayout(const QString& key, int listIndex, const QString& listKey, const ResourcePair& value) :
    QHBoxLayout(),
    _key(key),
    _listIndex(listIndex),
    _listKey(listKey),
    _value(value)
{
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setContentsMargins(0, 0, 0, 0);

    createCheckboxes();
}

TemplateResourceLayout::~TemplateResourceLayout()
{
    cleanupLayout();
}

void TemplateResourceLayout::handleResourceChanged()
{
    _value.first = 0;

    for(int i = 0; i < count(); ++i)
    {
        if(QLayoutItem *child = itemAt(i))
        {
            if(QCheckBox* checkBox = qobject_cast<QCheckBox*>(child->widget()))
            {
                if(checkBox->isChecked())
                    ++_value.first;
            }
        }
    }

    emitChange();
}

void TemplateResourceLayout::handleAddResource()
{
    emit addResource();
}

void TemplateResourceLayout::handleRemoveResource()
{
    emit removeResource(this);
}

void TemplateResourceLayout::handleEditResource()
{
    bool ok;
    int newValue = QInputDialog::getInt(nullptr,
                                        tr("Edit Resource Value"),
                                        tr("Number of Checkboxes:"),
                                        _value.second,
                                        0,
                                        100,
                                        1,
                                        &ok);
    if((ok) && (newValue != _value.second))
    {
        _value.second = newValue;
        if(_value.first > newValue)
            _value.first = newValue;
        cleanupLayout();
        createCheckboxes();
        emitChange();
    }
}

bool TemplateResourceLayout::eventFilter(QObject *object, QEvent *event)
{
    // Edit the number of checkboxes in the layout on right click
    if((event) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton)
        {
            QMenu* popupMenu = new QMenu();

            QAction* editItem = new QAction(QString("Edit..."), popupMenu);
            connect(editItem, &QAction::triggered, this, &TemplateResourceLayout::handleEditResource);
            popupMenu->addAction(editItem);

            if(_listIndex >= 0)
            {
                popupMenu->addSeparator();

                QAction* addItem = new QAction(QString("Add..."), popupMenu);
                connect(addItem, &QAction::triggered, this, &TemplateResourceLayout::handleAddResource);
                popupMenu->addAction(addItem);

                QAction* removeItem = new QAction(QString("Remove..."), popupMenu);
                connect(removeItem, &QAction::triggered, this, &TemplateResourceLayout::handleRemoveResource);
                popupMenu->addAction(removeItem);
            }

            popupMenu->popup(mouseEvent->globalPosition().toPoint());

            return true;
        }
    }

    return QHBoxLayout::eventFilter(object, event);
}

void TemplateResourceLayout::cleanupLayout()
{
    QLayoutItem *child;
    while((child = takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void TemplateResourceLayout::createCheckboxes()
{
    for(int i = 0; i < _value.second; ++i)
    {
        QCheckBox* checkBox = new QCheckBox();
        checkBox->setChecked(i < _value.first);
        addWidget(checkBox);
        connect(checkBox, &QCheckBox::stateChanged, this, &TemplateResourceLayout::handleResourceChanged);
    }
}

void TemplateResourceLayout::emitChange()
{
    if(_listIndex >= 0)
    {
        QVariant variantValue;
        variantValue.setValue(_value);
        emit resourceListValueChanged(_key, _listIndex, _listKey, variantValue);
    }
    else
    {
        emit resourceValueChanged(_key, _value);
    }
}
