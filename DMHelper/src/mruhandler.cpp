#include "mruhandler.h"
#include "mruaction.h"
#include <QMenu>

const int MRU_HANDLER_MAX_FILES = 99;

MRUHandler::MRUHandler(QMenu* actionsMenu, int mruCount, QObject *parent) :
    QObject(parent),
    _mruCount(0),
    _mruFiles(),
    _actionsMenu(actionsMenu)
{
    internalSetMRUCount(mruCount);
}

void MRUHandler::addMRUFile(const QString &mruFile)
{
    internalAddMRUFile(mruFile);
    updateMRUActions();
    emit mruListChanged();
}

int MRUHandler::getMRUCount() const
{
    return _mruCount;
}

void MRUHandler::setMRUCount(int mruCount)
{
    if(_mruCount == mruCount)
        return;

    internalSetMRUCount(mruCount);

    updateMRUActions();
    emit mruListChanged();
}

QStringList MRUHandler::getMRUList() const
{
    return _mruFiles;
}

void MRUHandler::setMRUList(const QStringList& mruList)
{
    if(_mruFiles == mruList)
        return;

    _mruFiles = mruList;
}

QMenu* MRUHandler::getActionsMenu() const
{
    return _actionsMenu;
}

void MRUHandler::setActionsMenu(QMenu* actionsMenu)
{
    if(actionsMenu != _actionsMenu)
    {
        if(_actionsMenu)
            _actionsMenu->clear();

        _actionsMenu = actionsMenu;
        updateMRUActions();
    }
}

void MRUHandler::readMRUFromSettings(QSettings& settings)
{
    QStringList fileList;
    QStringList::const_iterator fileIterator;
    QString keyName;
    QString fileName;

    // Create the MRU list from the registry
    settings.beginGroup("MRU");

        bool ok = true;
        int inputCount = settings.value("MRU Count").toInt(&ok);
        if(ok)
            internalSetMRUCount(inputCount);

        settings.beginGroup("MRU Files");
            fileList = settings.childKeys();
            // Add (push) the MRU files back-to-front
            for(int i = fileList.count() - 1; i >= 0; --i)
            {
                keyName = fileList.at(i).toLocal8Bit().constData();
                fileName = settings.value(keyName).toString();
                internalAddMRUFile(fileName);
            }
        settings.endGroup(); // MRU Files

    settings.endGroup(); // MRU

    updateMRUActions();
    emit mruListChanged();
}

void MRUHandler::writeMRUToSettings(QSettings& settings)
{
    // Create the MRU list in the registry
    settings.beginGroup("MRU");

        settings.remove("");
        settings.setValue("MRU Count", _mruCount);

        settings.beginGroup("MRU Files");
            QString keyName;
            for(int i = 0; i < _mruFiles.count(); ++i)
            {
                keyName = "MRU ";
                if((_mruCount >= 10) && (i < 10))
                {
                    keyName.append(QString("0"));
                }
                keyName.append(QString::number(i));
                settings.setValue(keyName, _mruFiles.at(i));
            }
        settings.endGroup(); // MRU Files

    settings.endGroup(); // MRU
}

void MRUHandler::updateMRUActions()
{
    if(_actionsMenu == nullptr)
        return;

    // Clear the list of existing MRU files
    _actionsMenu->clear();

    // Activate/Deactivate the menu item as needed
    _actionsMenu->setEnabled(_mruFiles.count() > 0);

    // Insert the new list of MRU files in front of the last separator
    QString actionStr;
    for(int i = 0; i < _mruFiles.count(); ++i)
    {
        actionStr = "&";
        actionStr.append(QString::number(i+1));
        actionStr.append(". ");
        actionStr.append(_mruFiles.at(i));
        MRUAction* newAction = new MRUAction(_mruFiles.at(i), actionStr, nullptr);
        if(newAction)
        {
            connect(newAction, SIGNAL(triggerFile(QString)), this, SIGNAL(triggerMRU(QString)));
            _actionsMenu->addAction(newAction);
        }
    }
}

void MRUHandler::internalSetMRUCount(int mruCount)
{
    _mruCount = qMin(qMax(0, mruCount), MRU_HANDLER_MAX_FILES);

    if(_mruCount == 0)
    {
        _mruFiles.clear();
    }
    else
    {
        // Cull any extra files in the list
        while(_mruFiles.count() >= _mruCount)
        {
            _mruFiles.pop_back();
        }
    }

    _mruFiles.reserve(_mruCount);
}

void MRUHandler::internalAddMRUFile(const QString &mruFile)
{
    // If the file is already in the MRU list, remove it and then push it
    int index = _mruFiles.indexOf(mruFile);
    if(index != -1)
    {
        _mruFiles.removeAt(index);
    }
    else
    {
        // Cull the MRU list to make space to add one more
        while(_mruFiles.count() >= _mruCount)
        {
            _mruFiles.pop_back();
        }
    }
    _mruFiles.push_front(mruFile);
}
