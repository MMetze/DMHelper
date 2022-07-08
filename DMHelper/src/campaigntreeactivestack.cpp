#include "campaigntreeactivestack.h"
#include <QDebug>

//#define DEBUG_ACTIVE_STACK

CampaignTreeActiveStack::CampaignTreeActiveStack(QObject *parent) :
    QObject{parent},
    _activeStack(),
    _index(-1)
{
}

void CampaignTreeActiveStack::backwards()
{
    if((_index <= 0) || (_index >= _activeStack.count()))
        return;

#ifdef DEBUG_ACTIVE_STACK
    qDebug() << "[CampaignTreeActiveStack] Moving backwards. Stack size: " << _activeStack.count() << ", new index: " << _index - 1 << ", UUID: " << _activeStack.at(_index - 1);
#endif
    emit activateItem(_activeStack.at(--_index));
    checkAvailable();
}

void CampaignTreeActiveStack::forwards()
{
    if((_index < 0) || (_index >= _activeStack.count() - 1))
        return;

#ifdef DEBUG_ACTIVE_STACK
    qDebug() << "[CampaignTreeActiveStack] Moving forwards. Stack size: " << _activeStack.count() << ", new index: " << _index + 1 << ", UUID: " << _activeStack.at(_index + 1);
#endif
    emit activateItem(_activeStack.at(++_index));
    checkAvailable();
}

void CampaignTreeActiveStack::push(const QUuid& uuid)
{
    // Don't do anything if the push is the current UUID
    if((_index >= 0) && (_index < _activeStack.count()) && (_activeStack.at(_index) == uuid))
    {
#ifdef DEBUG_ACTIVE_STACK
        qDebug() << "[CampaignTreeActiveStack] Push not doing anything since the pushed UUID is the same as the current UUID.";
#endif
        return;
    }

    // Remove all "future" entries
#ifdef DEBUG_ACTIVE_STACK
    qDebug() << "[CampaignTreeActiveStack] Clearing future stack to index " << _index << ", from total " << _activeStack.count();
#endif
    while(_index < _activeStack.count() - 1)
        _activeStack.removeLast();

    // Add the new uuid to the list
#ifdef DEBUG_ACTIVE_STACK
    qDebug() << "[CampaignTreeActiveStack] Pushing Uuid " << uuid << " to index " << _index;
#endif
    _activeStack.append(uuid);
    ++_index;
    checkAvailable();
}

void CampaignTreeActiveStack::clear()
{
#ifdef DEBUG_ACTIVE_STACK
    qDebug() << "[CampaignTreeActiveStack] Clearing stack";
#endif
    _index = -1;
    _activeStack.clear();
    checkAvailable();
}

void CampaignTreeActiveStack::checkAvailable()
{
    emit backwardsAvailable(_index > 0);
    emit forwardsAvailable((_index >= 0) && (_index < _activeStack.count() - 1));
}
