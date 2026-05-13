#include "templatefactory.h"
#include "templateobject.h"
#include "templateobjectnotifier.h"
#include "templatefieldformat.h"
#include "templateframe.h"
#include "templateresourcelayout.h"
#include "intfieldkeyhandler.h"
#include "dice.h"
#include "combatant.h"
#include "rulefactory.h"
#include <QVariant>
#include <QCoreApplication>
#include <QDomElement>
#include <QCoreApplication>
#include <QUiLoader>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QValidator>
#include <QScrollArea>
#include <QLayout>
#include <QLabel>
#include <QMessageBox>
#include <QBuffer>
#include <QFileInfo>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

const char* TemplateFactory::TEMPLATE_PROPERTY = "dmhValue";
const char* TemplateFactory::TEMPLATE_WIDGET = "dmhWidget";
const char* TemplateFactory::TEMPLATE_FORMAT = "dmhFormat";
const char* TemplateFactory::TEMPLATE_COMPUTE = "dmhCompute";
const char* TemplateFactory::TEMPLATE_CONDITION = "dmhCondition";
const char* TemplateFactory::TEMPLATE_DICE_MAXIMUM = "dmhDiceMaximum";
const char* TemplateFactory::TEMPLATE_DICE_AVERAGE = "dmhDiceAverage";

const char* TemplateFactory::TEMPLATEVALUES[TEMPLATETYPE_COUNT] =
    {
        "dmh:template",     // TemplateType_template
        "dmh:attribute",    // TemplateType_attribute
        "dmh:element",      // TemplateType_element
        "dmh:string",       // TemplateType_string
        "dmh:integer",      // TemplateType_integer
        "dmh:boolean",      // TemplateType_boolean
        "dmh:resource",     // TemplateType_resource
        "dmh:dice",         // TemplateType_dice
        "dmh:html",         // TemplateType_html
        "dmh:list",         // TemplateType_list
};

TemplateFactory::TemplateFactory(QObject *parent) :
    ObjectFactory{parent},
    _attributes(),
    _elements(),
    _elementLists(),
    _lineConnections(),
    _textConnections(),
    _otherConnections(),
    _computeConnections(),
    _conditionConnections()
{
}

QVariant TemplateFactory::convertStringToVariant(const QString& value, TemplateType type)
{
    switch(type)
    {
    case TemplateFactory::TemplateType_string:
        return QVariant(value);
    case TemplateFactory::TemplateType_integer:
        return QVariant(value.toInt());
    case TemplateFactory::TemplateType_boolean:
        return QVariant(value.toInt());
    case TemplateFactory::TemplateType_dice:
    {
        QVariant diceResult;
        diceResult.setValue(Dice(value));
        return diceResult;
    }
    case TemplateFactory::TemplateType_resource:
    {
        QStringList resourceList = value.split(",");
        if(resourceList.size() != 2)
        {
            qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to a resource pair, but it is not in the correct format";
            return QVariant();
        }

        QVariant resourceResult;
        resourceResult.setValue(ResourcePair(resourceList.at(0).toInt(), resourceList.at(1).toInt()));
        return resourceResult;
    }
    default:
        qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to the unexpected attribute type: " << type;
        return QVariant();
    }
}

QString TemplateFactory::convertVariantToString(const QVariant& value, TemplateType type)
{
    switch(type)
    {
    case TemplateFactory::TemplateType_string:
        return value.toString();
    case TemplateFactory::TemplateType_integer:
        return QString::number(value.toInt());
    case TemplateFactory::TemplateType_boolean:
        return QString::number(value.toInt());
    case TemplateFactory::TemplateType_dice:
        return value.value<Dice>().toString();
    case TemplateFactory::TemplateType_resource:
    {
        ResourcePair resourcePair = value.value<ResourcePair>();
        return QString::number(resourcePair.first) + QString(",") + QString::number(resourcePair.second);
    }
    default:
        qDebug() << "[TemplateFactory] WARNING: Trying to convert the value: " << value << " to the unexpected attribute type: " << type;
        return QString();
    }
}

QWidget* TemplateFactory::loadUITemplate(const QString& templateFile)
{
    QWidget* result = nullptr;

    if(templateFile.isEmpty())
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: No template file name provided, cannot load any UI template files";
        return result;
    }

    if(!RuleFactory::Instance())
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: No rule factory exists, cannot load any UI template files: " << templateFile;
        return result;
    }

    if(!QFileInfo::exists(templateFile))
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: Relative UI Template File not found: " << templateFile;
        return result;
    }

    // Cache the .ui file contents in memory keyed by absolute path + last
    // modification time. Each combatant in an encounter loads from the same
    // .ui file, so reading + parsing it from disk every time is wasteful.
    // Invalidating on mtime keeps live edits in Qt Designer working.
    struct CacheEntry { QByteArray data; qint64 mtime; };
    static QHash<QString, CacheEntry> cache;
    static QMutex cacheMutex;

    const QFileInfo info(templateFile);
    const QString cacheKey = info.absoluteFilePath();
    const qint64 currentMtime = info.lastModified().toMSecsSinceEpoch();

    QByteArray cachedData;
    {
        QMutexLocker locker(&cacheMutex);
        auto it = cache.constFind(cacheKey);
        if((it != cache.constEnd()) && (it->mtime == currentMtime))
            cachedData = it->data;
    }

    if(cachedData.isEmpty())
    {
        QFile file(templateFile);
        if(!file.open(QFile::ReadOnly))
        {
            qDebug() << "[RuleFactory::loadUITemplate] ERROR: Unable to read UI Template file: " << templateFile << ", error: " << file.error() << ", " << file.errorString();
            return result;
        }
        cachedData = file.readAll();
        file.close();

        QMutexLocker locker(&cacheMutex);
        cache.insert(cacheKey, { cachedData, currentMtime });
    }

    QUiLoader loader;
    QBuffer buffer(&cachedData);
    buffer.open(QIODevice::ReadOnly);
    result = loader.load(&buffer);

    if(!result)
    {
        qDebug() << "[RuleFactory::loadUITemplate] ERROR: UI Template File " << templateFile << " could not be loaded: " << loader.errorString();
        return result;
    }

    qDebug() << "[RuleFactory::loadUITemplate] UI Template File " << templateFile << " loaded.";

    // Activate hyperlinks for any included text edits
    QList<QTextEdit*> textEdits = result->findChildren<QTextEdit*>();
    for(QTextEdit* edit : textEdits)
    {
        auto &clist = edit->children();
        for(QObject *pObj : clist)
        {
            QString cname = pObj->metaObject()->className();
            if(cname == "QWidgetTextControl")
                pObj->setProperty("openExternalLinks", true);
        }
    }

    return result;
}

void TemplateFactory::readObjectData(QWidget* widget, TemplateObject* source, TemplateFrame* frame, QObject* filterObject)
{
    if((!widget) || (!frame))
        return;

    QList<QScrollArea*> scrollAreas = widget->findChildren<QScrollArea*>();

    // Clean up any existing scroll area children
    for(auto scrollArea : scrollAreas)
    {
        if(!scrollArea)
            continue;

        QString keyString = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        QString widgetString = scrollArea->property(TemplateFactory::TEMPLATE_WIDGET).toString();
        if((!keyString.isEmpty()) && (!widgetString.isEmpty()))
        {
            if(QWidget* oldWidget = scrollArea->takeWidget())
            {
                if(QLayout* oldLayout = oldWidget->layout())
                {
                    QLayoutItem *child;
                    while((child = oldLayout->takeAt(0)) != nullptr)
                    {
                        if(child->widget())
                            child->widget()->deleteLater();
                        delete child;
                    }
                    delete oldLayout;
                }
                oldWidget->deleteLater();
            }
        }
    }

    // Walk through the loaded UI Widget and allocate the appropriate values to the UI elements
    populateWidget(widget, source, frame);

    // Populate the scroll areas
    for(auto scrollArea : scrollAreas)
    {
        if(!scrollArea)
            continue;

        QString keyString = scrollArea->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        QString widgetString = scrollArea->property(TemplateFactory::TEMPLATE_WIDGET).toString();
        if((!keyString.isEmpty()) && (!widgetString.isEmpty()))
        {
            scrollArea->setWidgetResizable(true);
            scrollArea->setStyleSheet(QStringLiteral("QScrollArea { background: transparent; } QScrollArea > QWidget > QWidget { background: transparent; }"));
            QFrame* scrollWidget = new QFrame;
            QVBoxLayout* scrollLayout = new QVBoxLayout;
            scrollLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            scrollLayout->setContentsMargins(0, 0, 0, 0);
            scrollWidget->setLayout(scrollLayout);
            scrollArea->setWidget(scrollWidget);

            QList<QVariant> listValue = source->getListValue(keyString);
            if(!listValue.isEmpty())
            {
                for(int i = 0; i < listValue.count(); ++i)
                {
                    QVariant listEntry = listValue.at(i);
                    if(listEntry.isNull())
                        continue;

                    QWidget* newWidget = createResourceWidget(keyString, widgetString, frame->getUIFilename());
                    if(!newWidget)
                    {
                        qDebug() << "[TemplateFactory] ERROR: Unable to create the object widget: " << widgetString << ", for scroll area: " << keyString;
                        return;
                    }

                    QHash<QString, QVariant> hashValue = listEntry.toHash();
                    QHash<QString, DMHAttribute> hashAttributes = getElementList(keyString);

                    // Walk through the loaded UI Widget and allocate the appropriate object values to the UI elements
                    populateWidget(newWidget, source, frame, &hashValue, &hashAttributes, i, keyString);

                    newWidget->installEventFilter(filterObject);
                    scrollLayout->addWidget(newWidget);
                }
            }
            scrollArea->installEventFilter(filterObject);
        }
    }
}

void TemplateFactory::populateWidget(QWidget* widget, TemplateObject* source, TemplateFrame* templateFrame, QHash<QString, QVariant>* hash, QHash<QString, DMHAttribute>* hashAttributes, int listIndex, const QString& listKey)
{
    if((!widget) || (!templateFrame) || (!source))
        return;

    QList<QLineEdit*> lineEdits = widget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;

        QString keyString = lineEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        const QString computeSpec = lineEdit->property(TemplateFactory::TEMPLATE_COMPUTE).toString();
        const QString diceMaxKey = lineEdit->property(TemplateFactory::TEMPLATE_DICE_MAXIMUM).toString();
        const QString diceAvgKey = lineEdit->property(TemplateFactory::TEMPLATE_DICE_AVERAGE).toString();
        const QString formatSpec = lineEdit->property(TemplateFactory::TEMPLATE_FORMAT).toString();
        const FormatSpec parsedFormat = TemplateFieldFormat::parseFormat(formatSpec);

        // Dice-derived read-only fields: format the maximum or average of a dice
        // expression stored under another key on the source. Mutually exclusive
        // with dmhCompute and the writable dmhValue path; neither writes back.
        // Live-updates whenever the referenced key changes on the source.
        if((!diceMaxKey.isEmpty()) || (!diceAvgKey.isEmpty()))
        {
            const bool useMax = !diceMaxKey.isEmpty();
            const QString sourceKey = useMax ? diceMaxKey : diceAvgKey;
            lineEdit->setReadOnly(true);

            auto evaluate = [useMax, sourceKey, source]() -> int {
                const QString expr = source->getValueAsString(sourceKey);
                if(expr.isEmpty())
                    return 0;
                return useMax ? Dice::maximum(expr) : Dice::average(expr);
            };

            const int initialValue = evaluate();
            lineEdit->setText(TemplateFieldFormat::applyFormatInt(initialValue, parsedFormat));
            lineEdit->setCursorPosition(0);

            if(_computeConnections.contains(lineEdit))
                disconnect(_computeConnections[lineEdit]);

            auto diceConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, lineEdit,
                [lineEdit, parsedFormat, sourceKey, evaluate](const QString& changedKey) {
                    if(changedKey != sourceKey)
                        return;
                    lineEdit->setText(TemplateFieldFormat::applyFormatInt(evaluate(), parsedFormat));
                    lineEdit->setCursorPosition(0);
                });
            _computeConnections[lineEdit] = diceConn;
            continue;
        }

        // Computed (read-only) fields are independent of dmhValue: they evaluate
        // an expression against the source TemplateObject and live-update via the
        // notifier. They must not write back to the model.
        if(!computeSpec.isEmpty())
        {
            ComputeExpr expr = ComputeExpr::parse(computeSpec);
            if(expr.isValid())
            {
                lineEdit->setReadOnly(true);
                const int initialValue = expr.evaluate(*source);
                lineEdit->setText(TemplateFieldFormat::applyFormatInt(initialValue, parsedFormat));
                lineEdit->setCursorPosition(0);

                if(_computeConnections.contains(lineEdit))
                    disconnect(_computeConnections[lineEdit]);

                const QSet<QString> refs = expr.references();
                auto computeConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, lineEdit,
                    [lineEdit, expr, parsedFormat, refs, source](const QString& changedKey) {
                        if((!refs.isEmpty()) && (!refs.contains(changedKey)))
                            return;
                        const int v = expr.evaluate(*source);
                        lineEdit->setText(TemplateFieldFormat::applyFormatInt(v, parsedFormat));
                        lineEdit->setCursorPosition(0);
                    });
                _computeConnections[lineEdit] = computeConn;
                continue;
            }
            else
            {
                qDebug() << "[TemplateFactory] WARNING: Invalid dmhCompute expression on widget" << lineEdit->objectName() << ":" << computeSpec;
            }
        }

        if(!keyString.isEmpty())
        {
            QString valueString;
            if(hash)
            {
                if(hashAttributes)
                    valueString = convertVariantToString(hash->value(keyString), hashAttributes->value(keyString)._type);
                else
                    valueString = hash->value(keyString).toString();
            }
            else
            {
                valueString = source->getValueAsString(keyString);
            }

            if(valueString.isEmpty())
                valueString = getDefaultValue(keyString);

            lineEdit->setText(TemplateFieldFormat::applyFormat(valueString, parsedFormat));
            lineEdit->setCursorPosition(0);

            if(QValidator* validator = TemplateFieldFormat::makeValidator(parsedFormat, lineEdit))
                lineEdit->setValidator(validator);

            // Integer fields get arrow-key / +/- nudge support so the user can
            // bump values up and down by one without retyping or reaching for
            // the mouse. The handler is parented to the line edit so its
            // lifetime tracks the widget; only attach once per widget.
            if((parsedFormat.isInt) && (!lineEdit->findChild<IntFieldKeyHandler*>(QString(), Qt::FindDirectChildrenOnly)))
                new IntFieldKeyHandler(lineEdit, parsedFormat);

            if(_lineConnections.contains(lineEdit))
                disconnect(_lineConnections[lineEdit]);

            auto connection = connect(lineEdit, &QLineEdit::editingFinished, lineEdit,
                [lineEdit, templateFrame, source, parsedFormat]() {
                    const QString stripped = TemplateFieldFormat::stripFormat(lineEdit->text(), parsedFormat);
                    templateFrame->handleEditBoxChange(lineEdit, source, stripped);
                });
            _lineConnections[lineEdit] = connection;

            // Reverse binding: refresh on model change. Only meaningful when
            // the widget is bound directly to the source (not a list entry).
            if(!hash)
            {
                if(_reverseConnections.contains(lineEdit))
                    disconnect(_reverseConnections[lineEdit]);
                auto reverseConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, lineEdit,
                    [lineEdit, source, keyString, parsedFormat](const QString& changedKey) {
                        if(changedKey != keyString)
                            return;
                        const QString fresh = source->getValueAsString(keyString);
                        const QString formatted = TemplateFieldFormat::applyFormat(fresh, parsedFormat);
                        if(lineEdit->text() == formatted)
                            return;
                        QSignalBlocker block(lineEdit);
                        lineEdit->setText(formatted);
                        lineEdit->setCursorPosition(0);
                    });
                _reverseConnections[lineEdit] = reverseConn;
            }
        }
    }

    // QComboBox: store currentText (string) on change
    QList<QComboBox*> comboBoxes = widget->findChildren<QComboBox*>();
    for(auto comboBox : comboBoxes)
    {
        if(!comboBox)
            continue;

        const QString keyString = comboBox->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        QString valueString;
        if(hash)
        {
            if(hashAttributes)
                valueString = convertVariantToString(hash->value(keyString), hashAttributes->value(keyString)._type);
            else
                valueString = hash->value(keyString).toString();
        }
        else
        {
            valueString = source->getValueAsString(keyString);
        }
        if(valueString.isEmpty())
            valueString = getDefaultValue(keyString);

        // Disconnect any prior write-back connection BEFORE updating the widget
        // value, otherwise programmatic setCurrentIndex / setCurrentText will fire
        // the previous lambda (still wired to a stale source) and falsely mark
        // that source dirty.
        if(_otherConnections.contains(comboBox))
        {
            disconnect(_otherConnections[comboBox]);
            _otherConnections.remove(comboBox);
        }

        {
            QSignalBlocker blocker(comboBox);
            const int existingIndex = comboBox->findText(valueString);
            if(existingIndex >= 0)
                comboBox->setCurrentIndex(existingIndex);
            else if(comboBox->isEditable())
                comboBox->setCurrentText(valueString);
        }

        auto conn = connect(comboBox, &QComboBox::currentTextChanged, comboBox,
            [comboBox, templateFrame, source](const QString& text) {
                templateFrame->handleEditBoxChange(comboBox, source, text);
            });
        _otherConnections[comboBox] = conn;

        if(!hash)
        {
            if(_reverseConnections.contains(comboBox))
                disconnect(_reverseConnections[comboBox]);
            auto reverseConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, comboBox,
                [comboBox, source, keyString](const QString& changedKey) {
                    if(changedKey != keyString)
                        return;
                    const QString fresh = source->getValueAsString(keyString);
                    if(comboBox->currentText() == fresh)
                        return;
                    QSignalBlocker block(comboBox);
                    const int idx = comboBox->findText(fresh);
                    if(idx >= 0)
                        comboBox->setCurrentIndex(idx);
                    else if(comboBox->isEditable())
                        comboBox->setCurrentText(fresh);
                });
            _reverseConnections[comboBox] = reverseConn;
        }
    }

    // QSpinBox: integer-valued; write back as the integer's string form
    QList<QSpinBox*> spinBoxes = widget->findChildren<QSpinBox*>();
    for(auto spinBox : spinBoxes)
    {
        if(!spinBox)
            continue;

        const QString keyString = spinBox->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        int intValue = 0;
        if(hash)
            intValue = hash->value(keyString).toInt();
        else
            intValue = source->getIntValue(keyString);

        // Disconnect any prior write-back connection BEFORE setValue, otherwise
        // the previous lambda (still bound to a stale source) fires and falsely
        // marks that source dirty.
        if(_otherConnections.contains(spinBox))
        {
            disconnect(_otherConnections[spinBox]);
            _otherConnections.remove(spinBox);
        }

        {
            QSignalBlocker blocker(spinBox);
            spinBox->setValue(intValue);
        }

        auto conn = connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), spinBox,
            [spinBox, templateFrame, source](int v) {
                templateFrame->handleEditBoxChange(spinBox, source, QString::number(v));
            });
        _otherConnections[spinBox] = conn;

        if(!hash)
        {
            if(_reverseConnections.contains(spinBox))
                disconnect(_reverseConnections[spinBox]);
            auto reverseConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, spinBox,
                [spinBox, source, keyString](const QString& changedKey) {
                    if(changedKey != keyString)
                        return;
                    const int fresh = source->getIntValue(keyString);
                    if(spinBox->value() == fresh)
                        return;
                    QSignalBlocker block(spinBox);
                    spinBox->setValue(fresh);
                });
            _reverseConnections[spinBox] = reverseConn;
        }
    }

    // QCheckBox: boolean (1 or 0); skip checkboxes embedded inside resource frames
    QList<QCheckBox*> checkBoxes = widget->findChildren<QCheckBox*>();
    for(auto checkBox : checkBoxes)
    {
        if(!checkBox)
            continue;

        const QString keyString = checkBox->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        bool boolValue = false;
        if(hash)
            boolValue = hash->value(keyString).toBool();
        else
            boolValue = source->getBoolValue(keyString);

        QSignalBlocker blocker(checkBox);
        checkBox->setChecked(boolValue);
        blocker.unblock();

        if(_otherConnections.contains(checkBox))
            disconnect(_otherConnections[checkBox]);

        auto conn = connect(checkBox, &QCheckBox::toggled, checkBox,
            [checkBox, templateFrame, source](bool checked) {
                templateFrame->handleEditBoxChange(checkBox, source, checked ? QStringLiteral("1") : QStringLiteral("0"));
            });
        _otherConnections[checkBox] = conn;

        if(!hash)
        {
            if(_reverseConnections.contains(checkBox))
                disconnect(_reverseConnections[checkBox]);
            auto reverseConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, checkBox,
                [checkBox, source, keyString](const QString& changedKey) {
                    if(changedKey != keyString)
                        return;
                    const bool fresh = source->getBoolValue(keyString);
                    if(checkBox->isChecked() == fresh)
                        return;
                    QSignalBlocker block(checkBox);
                    checkBox->setChecked(fresh);
                });
            _reverseConnections[checkBox] = reverseConn;
        }
    }

    QList<QTextEdit*> textEdits = widget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if(!textEdit)
            continue;

        QString keyString = textEdit->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(!keyString.isEmpty())
        {
            QString valueString;
            if(hash)
            {
                if(hashAttributes)
                    valueString = convertVariantToString(hash->value(keyString), hashAttributes->value(keyString)._type);
                else
                    valueString = hash->value(keyString).toString();
            }
            else
            {
                valueString = source->getValueAsString(keyString);
            }

            // Disconnect any prior write-back connection BEFORE setHtml, otherwise
            // the textChanged signal from the programmatic update will fire the
            // previous lambda (still bound to a stale source) and falsely mark
            // that source dirty.
            if(_textConnections.contains(textEdit))
            {
                disconnect(_textConnections[textEdit]);
                _textConnections.remove(textEdit);
            }

            {
                QSignalBlocker blocker(textEdit);
                textEdit->setHtml(valueString.isEmpty() ? getDefaultValue(keyString) : valueString);
                textEdit->moveCursor(QTextCursor::Start);
            }

            auto connection = connect(textEdit, &QTextEdit::textChanged, textEdit, [=]() { templateFrame->handleEditBoxChange(textEdit, source, textEdit->toPlainText().isEmpty() ? QString() : textEdit->toHtml()); });
            _textConnections[textEdit] = connection;

            if(!hash)
            {
                if(_reverseConnections.contains(textEdit))
                    disconnect(_reverseConnections[textEdit]);
                auto reverseConn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, textEdit,
                    [textEdit, source, keyString](const QString& changedKey) {
                        if(changedKey != keyString)
                            return;
                        const QString fresh = source->getValueAsString(keyString);
                        if(textEdit->toHtml() == fresh)
                            return;
                        QSignalBlocker block(textEdit);
                        textEdit->setHtml(fresh);
                        textEdit->moveCursor(QTextCursor::Start);
                    });
                _reverseConnections[textEdit] = reverseConn;
            }
        }
    }

    QList<QFrame*> frames = widget->findChildren<QFrame*>();
    for(auto frame : frames)
    {
        if((!frame) || (dynamic_cast<QTextEdit*>(frame)) || (dynamic_cast<QScrollArea*>(frame)) || (dynamic_cast<QScrollArea*>(frame->parentWidget())))
            continue;

        QString keyString = frame->property(TemplateFactory::TEMPLATE_PROPERTY).toString();
        if(keyString.isEmpty())
            continue;

        if(QLayout* oldLayout = frame->layout())
        {
            frame->removeEventFilter(oldLayout);
            delete oldLayout;
        }

        ResourcePair valuePair;
        if(hash)
            valuePair = hash->value(keyString).value<ResourcePair>();
        else
            valuePair = source->getResourceValue(keyString);

        TemplateResourceLayout* layout = nullptr;
        if(hash)
        {
            layout = new TemplateResourceLayout(listKey, listIndex, keyString, valuePair);
            connect(layout, &TemplateResourceLayout::resourceListValueChanged, [source](const QString& key, int listIndex, const QString& listKey, const QVariant& value){ source->setListValue(key, listIndex, listKey, value); });
            connect(layout, &TemplateResourceLayout::addResource, [templateFrame, source, widget](){ templateFrame->handleAddResource(widget, source); });
            connect(layout, &TemplateResourceLayout::removeResource, [templateFrame, source, widget](){ templateFrame->handleRemoveResource(widget, source); });
        }
        else
        {
            layout = new TemplateResourceLayout(keyString, valuePair);
            connect(layout, &TemplateResourceLayout::resourceValueChanged, [source](const QString& key, const ResourcePair& value){ source->setResourceValue(key, value); });
        }
        frame->installEventFilter(layout);
        frame->setLayout(layout);
    }

    // dmhCondition pass: drive widget visibility from sibling field values.
    // Syntax: "field op value" with op in ==, !=, >=, <=, >, < ; AND-join
    // multiple clauses with ';'. Numeric compare when both sides parse as
    // integers, else lexical compare. Hides widget when any clause fails.
    QList<QWidget*> condWidgets = widget->findChildren<QWidget*>();
    condWidgets.append(widget);
    for(QWidget* condWidget : condWidgets)
    {
        if(!condWidget)
            continue;
        const QString condSpec = condWidget->property(TemplateFactory::TEMPLATE_CONDITION).toString();
        if(condSpec.isEmpty())
            continue;

        struct Clause { QString field; QString op; QString value; };
        QList<Clause> clauses;
        QSet<QString> refs;
        const QStringList rawClauses = condSpec.split(QChar(';'), Qt::SkipEmptyParts);
        for(const QString& raw : rawClauses)
        {
            const QString clauseStr = raw.trimmed();
            if(clauseStr.isEmpty())
                continue;
            static const QStringList ops = { QStringLiteral("=="), QStringLiteral("!="),
                                             QStringLiteral(">="), QStringLiteral("<="),
                                             QStringLiteral(">"),  QStringLiteral("<") };
            int opIdx = -1;
            QString opStr;
            for(const QString& op : ops)
            {
                const int idx = clauseStr.indexOf(op);
                if(idx > 0)
                {
                    opIdx = idx;
                    opStr = op;
                    break;
                }
            }
            if(opIdx < 0)
            {
                qDebug() << "[TemplateFactory] WARNING: Invalid dmhCondition clause on widget" << condWidget->objectName() << ":" << clauseStr;
                continue;
            }
            Clause c;
            c.field = clauseStr.left(opIdx).trimmed();
            c.op = opStr;
            c.value = clauseStr.mid(opIdx + opStr.length()).trimmed();
            if(c.field.isEmpty())
                continue;
            clauses.append(c);
            refs.insert(c.field);
        }
        if(clauses.isEmpty())
            continue;

        auto evaluate = [clauses, source, hash, hashAttributes]() -> bool {
            for(const Clause& c : clauses)
            {
                QString lhs;
                if(hash)
                {
                    if(hashAttributes)
                        lhs = convertVariantToString(hash->value(c.field), hashAttributes->value(c.field)._type);
                    else
                        lhs = hash->value(c.field).toString();
                }
                else
                {
                    lhs = source->getValueAsString(c.field);
                }

                bool lhsOk = false, rhsOk = false;
                const int lhsInt = lhs.toInt(&lhsOk);
                const int rhsInt = c.value.toInt(&rhsOk);
                bool result = false;
                if(lhsOk && rhsOk)
                {
                    if(c.op == QLatin1String("==")) result = (lhsInt == rhsInt);
                    else if(c.op == QLatin1String("!=")) result = (lhsInt != rhsInt);
                    else if(c.op == QLatin1String(">"))  result = (lhsInt >  rhsInt);
                    else if(c.op == QLatin1String("<"))  result = (lhsInt <  rhsInt);
                    else if(c.op == QLatin1String(">=")) result = (lhsInt >= rhsInt);
                    else if(c.op == QLatin1String("<=")) result = (lhsInt <= rhsInt);
                }
                else
                {
                    const int cmp = QString::compare(lhs, c.value);
                    if(c.op == QLatin1String("==")) result = (cmp == 0);
                    else if(c.op == QLatin1String("!=")) result = (cmp != 0);
                    else if(c.op == QLatin1String(">"))  result = (cmp >  0);
                    else if(c.op == QLatin1String("<"))  result = (cmp <  0);
                    else if(c.op == QLatin1String(">=")) result = (cmp >= 0);
                    else if(c.op == QLatin1String("<=")) result = (cmp <= 0);
                }
                if(!result)
                    return false;
            }
            return true;
        };

        condWidget->setVisible(evaluate());

        if(_conditionConnections.contains(condWidget))
            disconnect(_conditionConnections[condWidget]);

        // Live updates only available against the model notifier (not list hashes).
        if(!hash)
        {
            auto conn = connect(source->notifier(), &TemplateObjectNotifier::valueChanged, condWidget,
                [condWidget, refs, evaluate](const QString& changedKey) {
                    if((!refs.isEmpty()) && (!refs.contains(changedKey)))
                        return;
                    condWidget->setVisible(evaluate());
                });
            _conditionConnections[condWidget] = conn;
        }
    }
}

QWidget* TemplateFactory::createResourceWidget(const QString& keyString, const QString& widgetString, const QString& templateFile)
{
    if(widgetString.isEmpty())
        return createResourceWidgetInternal(keyString);

    QString appFile;
    if(!templateFile.isEmpty())
    {
        QFileInfo fileInfo(templateFile);
        appFile = fileInfo.absolutePath();
        appFile += QString("/") + widgetString;
    }

    if(QFileInfo::exists(appFile))
    {
        return createResourceWidgetFile(appFile);
    }
    else
    {
        qDebug() << "[TemplateFactory] ERROR: UI Widget Template File not found: " << appFile << ", for the widget name: " << widgetString;
        return createResourceWidgetInternal(keyString);
    }
}

void TemplateFactory::disconnectWidget(QWidget* widget)
{
    if(!widget)
        return;

    // Walk through the loaded UI Widget and allocate the appropriate character values to the UI elements
    QList<QLineEdit*> lineEdits = widget->findChildren<QLineEdit*>();
    for(auto lineEdit : lineEdits)
    {
        if(!lineEdit)
            continue;
        if(_lineConnections.contains(lineEdit))
        {
            disconnect(_lineConnections[lineEdit]);
            _lineConnections.remove(lineEdit);
        }
        if(_computeConnections.contains(lineEdit))
        {
            disconnect(_computeConnections[lineEdit]);
            _computeConnections.remove(lineEdit);
        }
    }

    QList<QTextEdit*> textEdits = widget->findChildren<QTextEdit*>();
    for(auto textEdit : textEdits)
    {
        if((textEdit) && (_textConnections.contains(textEdit)))
        {
            disconnect(_textConnections[textEdit]);
            _textConnections.remove(textEdit);
        }
    }

    const QList<QWidget*> others = widget->findChildren<QWidget*>();
    for(auto other : others)
    {
        if((other) && (_otherConnections.contains(other)))
        {
            disconnect(_otherConnections[other]);
            _otherConnections.remove(other);
        }
        if((other) && (_conditionConnections.contains(other)))
        {
            disconnect(_conditionConnections[other]);
            _conditionConnections.remove(other);
        }
        if((other) && (_reverseConnections.contains(other)))
        {
            disconnect(_reverseConnections[other]);
            _reverseConnections.remove(other);
        }
    }
    if(_conditionConnections.contains(widget))
    {
        disconnect(_conditionConnections[widget]);
        _conditionConnections.remove(widget);
    }
}

bool TemplateFactory::isEmpty() const
{
    return _attributes.isEmpty() && _elements.isEmpty() && _elementLists.isEmpty();
}

bool TemplateFactory::hasAttribute(const QString& name) const
{
    return _attributes.contains(name);
}

DMHAttribute TemplateFactory::getAttribute(const QString& name) const
{
    return _attributes.value(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getAttributes() const
{
    return _attributes;
}

bool TemplateFactory::hasElement(const QString& name) const
{
    return _elements.contains(name);
}

DMHAttribute TemplateFactory::getElement(const QString& name) const
{
    return _elements.value(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getElements() const
{
    return _elements;
}

bool TemplateFactory::hasElementList(const QString& name) const
{
    return _elementLists.contains(name);
}

QHash<QString, DMHAttribute> TemplateFactory::getElementList(const QString& name) const
{
    return _elementLists.value(name);
}

QHash<QString, QHash<QString, DMHAttribute>> TemplateFactory::getElementLists() const
{
    return _elementLists;
}

bool TemplateFactory::hasEntry(const QString& name) const
{
    return hasAttribute(name) || hasElement(name) || hasElementList(name);
}

QString TemplateFactory::getDefaultValue(const QString& keyString)
{
    if(hasAttribute(keyString))
        return getAttribute(keyString)._default;
    else if(hasElement(keyString))
        return getElement(keyString)._default;
    else
        return QString();
}

TemplateObject* TemplateFactory::setDefaultValues(TemplateObject* object)
{
    if(!object)
        return nullptr;

    QHash<QString, DMHAttribute> attributes = getAttributes();
    for(auto it = attributes.begin(); it != attributes.end(); ++it)
    {
        object->setValue(it.key(), it->_default);
    }

    QHash<QString, DMHAttribute> elements = getElements();
    for(auto it = elements.begin(); it != elements.end(); ++it)
    {
        object->setValue(it.key(), it->_default);
    }

    return object;
}

QString TemplateFactory::getAbsoluteTemplateFile(const QString& templateFile)
{
    // Try our best to load the given template file
    QString appFile;

    if(QFileInfo(templateFile).isRelative())
    {
        appFile = QDir::current().absoluteFilePath(templateFile);
        if(!QFileInfo::exists(appFile))
        {
#ifdef Q_OS_MAC
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            fileDirPath.cdUp();
            appFile = fileDirPath.path() + QString("/Resources/") + templateFile;
#else
            QDir fileDirPath(QCoreApplication::applicationDirPath());
            appFile = fileDirPath.path() + QString("/resources/") + templateFile;
#endif
        }
    }
    else
    {
        appFile = templateFile;
    }

    if(!QFileInfo::exists(appFile))
    {
        qDebug() << "[TemplateFactory] ERROR: Template File not found: " << templateFile;
        return QString();
    }

    return appFile;
}

void TemplateFactory::loadTemplate(const QString& templateFile)
{
    if(!RuleFactory::Instance())
    {
        qDebug() << "[TemplateFactory] ERROR: No rule factory exists, cannot load the template file: " << templateFile;
        return;
    }

    QString absoluteTemplateFile = getAbsoluteTemplateFile(templateFile);
    if(absoluteTemplateFile.isEmpty())
    {
        qDebug() << "[TemplateFactory] ERROR: Unable to find the template file: " << templateFile;
        return;
    }

    qDebug() << "[TemplateFactory] Reading Template File " << absoluteTemplateFile;

    QDomDocument doc;
    QFile file(absoluteTemplateFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[TemplateFactory] Template file open failed: " << absoluteTemplateFile << ", error: " << file.error() << ", " << file.errorString();
        QMessageBox::critical(nullptr, QString("Template file open failed"), QString("Unable to open the template file: ") + " - " + file.errorString());
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QDomDocument::ParseResult contentResult = doc.setContent(in.readAll());

    file.close();

    if(!contentResult)
    {
        qDebug() << "[TemplateFactory] Error reading template XML content. The XML is probably not valid at line " << contentResult.errorLine << ", column " << contentResult.errorColumn << ": " << contentResult.errorMessage;
        QMessageBox::critical(nullptr, QString("Template invalid"), QString("Unable to read the template: ") + absoluteTemplateFile + QString(", the XML is invalid"));
        return;
    }

    _attributes.clear();
    _elements.clear();
    _elementLists.clear();

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != TEMPLATEVALUES[TemplateType_template]))
    {
        qDebug() << "[TemplateFactory] Ttemplate missing root item: " << absoluteTemplateFile;
        QMessageBox::critical(nullptr, QString("Template file invalid"), QString("Unable to read the template: ") + absoluteTemplateFile + QString(", the XML does not have the expected root item."));
        return;
    }

    QDomElement element = root.firstChildElement();
    while(!element.isNull())
    {
        QString elementName = element.attribute("name");
        QString elementType = element.attribute("type");
        if(element.tagName() == TEMPLATEVALUES[TemplateType_attribute])
        {
            if(elementType == TEMPLATEVALUES[TemplateType_string])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_string, element.attribute("default")));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_integer])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_integer, element.attribute("default", QString("0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_boolean])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_boolean, element.attribute("default", QString("0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_resource])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_resource, element.attribute("default", QString("0,0"))));
            }
            if(elementType == TEMPLATEVALUES[TemplateType_dice])
            {
                _attributes.insert(elementName, DMHAttribute(TemplateType_dice, element.attribute("default")));
            }
        }
        else if(element.tagName() == TEMPLATEVALUES[TemplateType_element])
        {
            if(elementType == TEMPLATEVALUES[TemplateType_html])
            {
                _elements.insert(elementName, DMHAttribute(TemplateType_html, element.attribute("default")));
            }
            else if(elementType == TEMPLATEVALUES[TemplateType_list])
            {
                QHash<QString, DMHAttribute> newHash;
                QDomElement listElement = element.firstChildElement();
                while(!listElement.isNull())
                {
                    QString listElementName = listElement.attribute("name");
                    QString listElementType = listElement.attribute("type");
                    if(listElementType == TEMPLATEVALUES[TemplateType_string])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_string, listElement.attribute("default")));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_integer])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_integer, listElement.attribute("default", QString("0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_boolean])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_boolean, listElement.attribute("default", QString("0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_resource])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_resource, listElement.attribute("default", QString("0,0"))));
                    }
                    else if(listElementType == TEMPLATEVALUES[TemplateType_dice])
                    {
                        newHash.insert(listElementName, DMHAttribute(TemplateType_dice, listElement.attribute("default")));
                    }

                    listElement = listElement.nextSiblingElement();
                }

                _elementLists.insert(elementName, newHash);
            }
        }

        element = element.nextSiblingElement();
    }

    // Validate that each entry has a name attribute
    if(!hasAttribute(QString("name")))
        qDebug() << "[TemplateFactory] ERROR: Template has no 'name' attribute";
}

QWidget* TemplateFactory::createResourceWidgetFile(const QString& widgetFilename)
{
    if(widgetFilename.isEmpty())
        return nullptr;

    QUiLoader loader;
    QFile file(widgetFilename);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << "[TemplateFactory] ERROR: Unable to open UI Widget Template File: " << widgetFilename;
        return nullptr;
    }

    QWidget* newWidget = loader.load(&file);
    file.close();

    if(!newWidget)
        qDebug() << "[TemplateFactory] ERROR: UI Widget Template File could not be loaded: " << widgetFilename << ", error: " << loader.errorString();

    return newWidget;
}

QWidget* TemplateFactory::createResourceWidgetInternal(const QString& keyString)
{
    QHash<QString, DMHAttribute> elementList = getElementList(keyString);
    if(elementList.isEmpty())
        return nullptr;

    // Interate through the keys of the elementList and create the appropriate UI elements
    QWidget* newWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(newWidget);

    QStringList keys = elementList.keys();
    for(const auto &elementName : keys)
    {
        if(elementName.isEmpty())
            continue;

        layout->addWidget(new QLabel(elementName));
        QWidget* lineEditWidget = new QLineEdit();
        lineEditWidget->setProperty(TemplateFactory::TEMPLATE_PROPERTY, keyString);
        layout->addWidget(lineEditWidget);
    }

    return newWidget;
}
