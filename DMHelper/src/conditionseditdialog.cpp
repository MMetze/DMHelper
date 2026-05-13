#include "conditionseditdialog.h"
#include "ui_conditionseditdialog.h"
#include "conditions.h"
#include "ribbonframe.h"
#include "quickref.h"
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDebug>

const int CONDITION_GRID_COLUMNS = 5;

ConditionsEditDialog::ConditionsEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConditionsEditDialog),
    _entries(),
    _groupButtons(),
    _groupFrames()
{
    ui->setupUi(this);
    populateConditions();
}

ConditionsEditDialog::~ConditionsEditDialog()
{
    delete ui;
}

void ConditionsEditDialog::populateConditions()
{
    Conditions* conditions = Conditions::activeConditions();
    if(!conditions)
    {
        qDebug() << "[ConditionsEditDialog] ERROR: No active conditions available, dialog will be blank";
        return;
    }

    QList<ConditionDefinition> condDefs = conditions->getConditions();
    qDebug() << "[ConditionsEditDialog] Building UI with" << condDefs.count() << "condition definitions";

    // Separate non-grouped and grouped definitions
    QList<ConditionDefinition> ungrouped;
    QMap<QString, QList<ConditionDefinition>> groups; // ordered by first appearance
    for(const ConditionDefinition& def : condDefs)
    {
        if(def.group.isEmpty())
            ungrouped.append(def);
        else
            groups[def.group].append(def);
    }

    // Pass 1: lay out non-grouped conditions in the grid
    int row = 0;
    int col = 0;
    for(const ConditionDefinition& def : ungrouped)
    {
        QPushButton* btn = new QPushButton(ui->scrollAreaWidgetContents);
        btn->setCheckable(true);
        btn->setChecked(false);
        btn->setFlat(true);
        btn->setAutoDefault(false);

        QString iconPath = conditions->getConditionIconPath(def.id);
        if(!iconPath.isEmpty())
            btn->setIcon(QIcon(iconPath));

        setConditionTooltip(*btn, def.id);

        QLabel* label = new QLabel(def.title, ui->scrollAreaWidgetContents);
        label->setAlignment(Qt::AlignHCenter);

        ui->conditionGrid->addWidget(btn, row, col);
        ui->conditionGrid->addWidget(label, row + 1, col);

        ConditionEntry entry;
        entry.id = def.id;
        entry.group = QString();
        entry.button = btn;
        entry.label = label;
        _entries.append(entry);

        col++;
        if(col >= CONDITION_GRID_COLUMNS)
        {
            col = 0;
            row += 2;
        }
    }

    // Advance past the last non-grouped row
    if(col != 0)
        row += 2;

    // Pass 2: lay out each group as a framed section below the grid
    for(auto it = groups.begin(); it != groups.end(); ++it)
    {
        const QString& groupName = it.key();
        const QList<ConditionDefinition>& defs = it.value();
        if(defs.isEmpty())
            continue;

        // Group frame with a visible border
        QFrame* groupFrame = new QFrame(ui->scrollAreaWidgetContents);
        groupFrame->setFrameShape(QFrame::StyledPanel);
        groupFrame->setFrameShadow(QFrame::Raised);
        groupFrame->setStyleSheet(QStringLiteral("background: transparent;"));
        QHBoxLayout* frameLayout = new QHBoxLayout(groupFrame);
        frameLayout->setContentsMargins(4, 4, 4, 4);
        frameLayout->setSpacing(6);
        _groupFrames[groupName] = groupFrame;

        // Master toggle: icon button + label stacked vertically
        QWidget* masterWidget = new QWidget(groupFrame);
        QVBoxLayout* masterLayout = new QVBoxLayout(masterWidget);
        masterLayout->setContentsMargins(0, 0, 0, 0);
        masterLayout->setSpacing(2);

        QPushButton* groupBtn = new QPushButton(masterWidget);
        groupBtn->setCheckable(true);
        groupBtn->setChecked(false);
        groupBtn->setFlat(true);
        groupBtn->setAutoDefault(false);
        groupBtn->setStyleSheet(QStringLiteral(
            "QPushButton { border: 1px solid transparent; background: transparent; }"
            "QPushButton:checked { border: 2px solid rgb(115, 18, 0); background-color: rgba(50, 50, 50, 50); border-radius: 4px; }"
        ));
        QString iconPath = conditions->getConditionIconPath(defs.first().id);
        if(!iconPath.isEmpty())
            groupBtn->setIcon(QIcon(iconPath));

        _groupButtons[groupName] = groupBtn;
        masterLayout->addWidget(groupBtn, 0, Qt::AlignHCenter);

        QString displayName = groupName.at(0).toUpper() + groupName.mid(1);
        QLabel* groupLabel = new QLabel(displayName, masterWidget);
        groupLabel->setAlignment(Qt::AlignHCenter);
        masterLayout->addWidget(groupLabel);

        frameLayout->addWidget(masterWidget, 0, Qt::AlignVCenter);

        // Sub-buttons for each level
        for(const ConditionDefinition& def : defs)
        {
            // Derive a short label: strip the group prefix ("Exhaustion - Level 1" -> "Level 1")
            QString shortLabel = def.description;
            QString prefix = displayName + QStringLiteral(" - ");
            if(shortLabel.startsWith(prefix, Qt::CaseInsensitive))
                shortLabel = shortLabel.mid(prefix.length());

            QPushButton* btn = new QPushButton(shortLabel, groupFrame);
            btn->setCheckable(true);
            btn->setChecked(false);
            btn->setAutoDefault(false);
            btn->setEnabled(false);
            btn->setStyleSheet(QStringLiteral(
                "QPushButton { border: 1px solid transparent; background: transparent; }"
                "QPushButton:checked { border: 1px solid rgb(115, 18, 0); background-color: rgba(50, 50, 50, 50); border-radius: 2px; }"
                ));

            setConditionTooltip(*btn, def.id);
            frameLayout->addWidget(btn);

            ConditionEntry entry;
            entry.id = def.id;
            entry.group = groupName;
            entry.button = btn;
            entry.label = nullptr;
            _entries.append(entry);
        }

        frameLayout->addStretch();

        // Enable/disable sub-buttons when master is toggled
        connect(groupBtn, &QPushButton::toggled, this, [this, groupName](bool checked) {
            for(ConditionEntry& entry : _entries)
            {
                if(entry.group == groupName)
                    entry.button->setEnabled(checked);
            }
        });

        ui->conditionGrid->addWidget(groupFrame, row, 0, 1, CONDITION_GRID_COLUMNS);
        row++;
    }
}

void ConditionsEditDialog::setConditionList(const QStringList& conditions)
{
    // Uncheck everything
    for(ConditionEntry& entry : _entries)
        entry.button->setChecked(false);
    for(auto it = _groupButtons.begin(); it != _groupButtons.end(); ++it)
        it.value()->setChecked(false);

    // Determine which groups need enabling, then enable them first
    QSet<QString> activeGroups;
    for(const QString& id : conditions)
    {
        for(const ConditionEntry& entry : _entries)
        {
            if(entry.id == id && !entry.group.isEmpty())
            {
                activeGroups.insert(entry.group);
                break;
            }
        }
    }

    // Enable group masters (this triggers the toggled signal, which enables sub-buttons)
    for(const QString& group : activeGroups)
    {
        if(_groupButtons.contains(group))
            _groupButtons[group]->setChecked(true);
    }

    // Now check the individual condition buttons
    for(const QString& id : conditions)
    {
        for(ConditionEntry& entry : _entries)
        {
            if(entry.id == id)
            {
                entry.button->setChecked(true);
                break;
            }
        }
    }
}

QStringList ConditionsEditDialog::getConditionList() const
{
    QStringList result;

    for(const ConditionEntry& entry : _entries)
    {
        if(!entry.button->isChecked())
            continue;

        if(!entry.group.isEmpty())
        {
            if(_groupButtons.contains(entry.group) && _groupButtons[entry.group]->isChecked())
                result.append(entry.id);
        }
        else
        {
            result.append(entry.id);
        }
    }

    return result;
}

void ConditionsEditDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    int ribbonHeight = RibbonFrame::getRibbonHeight();
    if(ribbonHeight <= 0)
        return;

    // Find the widest label text to use as the uniform width for all buttons and labels
    int maxLabelWidth = 0;
    QFontMetrics metrics = fontMetrics();
    for(const ConditionEntry& entry : _entries)
    {
        if(entry.label)
        {
            int w = metrics.horizontalAdvance(entry.label->text());
            if(w > maxLabelWidth)
                maxLabelWidth = w;
        }
    }

    int labelHeight = RibbonFrame::getLabelHeight(metrics, ribbonHeight);
    int iconDim = ribbonHeight - labelHeight;

    // Use the wider of the icon dimension and the widest label
    int cellWidth = qMax(iconDim, maxLabelWidth);

    // Apply uniform sizing to all non-grouped condition buttons and labels
    for(ConditionEntry& entry : _entries)
    {
        if(entry.label && entry.group.isEmpty())
        {
            RibbonFrame::setButtonSize(*entry.button, cellWidth, iconDim);
            entry.button->setIconSize(QSize(iconDim * 2 / 3, iconDim * 2 / 3));
            RibbonFrame::setWidgetSize(*entry.label, cellWidth, labelHeight);
        }
    }

    // Size group master buttons the same way
    for(auto it = _groupButtons.begin(); it != _groupButtons.end(); ++it)
    {
        RibbonFrame::setButtonSize(*it.value(), iconDim, iconDim);
        it.value()->setIconSize(QSize(iconDim * 2 / 3, iconDim * 2 / 3));
    }
}

void ConditionsEditDialog::setConditionTooltip(QPushButton& button, const QString& conditionId)
{
    Conditions* conditions = Conditions::activeConditions();
    if(!conditions)
        return;

    QString title = conditions->getConditionTitle(conditionId);
    QString conditionText = QString("<b>") + title + QString("</b>");

    if(QuickRef::Instance())
    {
        QuickRefData* conditionData = QuickRef::Instance()->getData(QString("Condition"), 0, title);
        if(conditionData)
            conditionText += QString("<p>") + conditionData->getOverview();
    }

    button.setToolTip(conditionText);
}
