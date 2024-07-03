#include "charactertemplateresourcelayout.h"
#include <QWidget>
#include <QCheckBox>
#include <QMouseEvent>
#include <QInputDialog>

CharacterTemplateResourceLayout::CharacterTemplateResourceLayout(const QString& key, const ResourcePair& value) :
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

CharacterTemplateResourceLayout::CharacterTemplateResourceLayout(const QString& key, int listIndex, const QString& listKey, const ResourcePair& value) :
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

CharacterTemplateResourceLayout::~CharacterTemplateResourceLayout()
{
    cleanupLayout();
}

void CharacterTemplateResourceLayout::handleResourceChanged()
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

bool CharacterTemplateResourceLayout::eventFilter(QObject *object, QEvent *event)
{
    // Edit the number of checkboxes in the layout on right click
    if((event) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->button() == Qt::RightButton)
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
    }

    return QHBoxLayout::eventFilter(object, event);
}

void CharacterTemplateResourceLayout::cleanupLayout()
{
    QLayoutItem *child;
    while((child = takeAt(0)) != nullptr)
    {
        if(child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

void CharacterTemplateResourceLayout::createCheckboxes()
{
    for(int i = 0; i < _value.second; ++i)
    {
        QCheckBox* checkBox = new QCheckBox();
        checkBox->setChecked(i < _value.first);
        addWidget(checkBox);
        connect(checkBox, &QCheckBox::stateChanged, this, &CharacterTemplateResourceLayout::handleResourceChanged);
    }
}

void CharacterTemplateResourceLayout::emitChange()
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
