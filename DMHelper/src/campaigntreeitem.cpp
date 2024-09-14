#include "campaigntreeitem.h"
#include "campaignobjectbase.h"
#include "dmconstants.h"
#include "characterv2.h"
#include "audiotrack.h"
#include <QUuid>

CampaignTreeItem::CampaignTreeItem() :
    QStandardItem()
{
}

CampaignTreeItem::CampaignTreeItem(int rows, int columns) :
    QStandardItem(rows, columns)
{
}

CampaignTreeItem::CampaignTreeItem(const QIcon &icon, const QString &text) :
    QStandardItem(icon, text)
{
}

CampaignTreeItem::CampaignTreeItem(const QString &text) :
    QStandardItem(text)
{
}

CampaignTreeItem::CampaignTreeItem(const CampaignTreeItem& other) :
    QStandardItem(other.icon(), other.text())
{
    setCampaignItemId(other.getCampaignItemId());
    setCampaignItemType(other.getCampaignItemType());
    setCampaignItemObject(other.getCampaignItemObject());
}

CampaignTreeItem::~CampaignTreeItem()
{
}

QStandardItem* CampaignTreeItem::clone() const
{
    //return QStandardItem::clone();
    CampaignTreeItem* newItem = new CampaignTreeItem(*this);
    return newItem;
}

int CampaignTreeItem::type() const
{
    return Qt::UserRole;
}

int CampaignTreeItem::getCampaignItemType() const
{
    return data(DMHelper::TreeItemData_Type).toInt();
}

void CampaignTreeItem::setCampaignItemType(int itemType)
{
    setData(QVariant(itemType), DMHelper::TreeItemData_Type);
}

QUuid CampaignTreeItem::getCampaignItemId() const
{
    QVariant v = data(DMHelper::TreeItemData_ID);
    QString s = v.toString();
    QUuid u(s);
    return u;
}

void CampaignTreeItem::setCampaignItemId(const QUuid& itemId)
{
    setData(QVariant(itemId.toString()), DMHelper::TreeItemData_ID);
}

CampaignObjectBase* CampaignTreeItem::getCampaignItemObject() const
{
    return reinterpret_cast<CampaignObjectBase*>(data(DMHelper::TreeItemData_Object).value<quint64>());
}

void CampaignTreeItem::setCampaignItemObject(CampaignObjectBase* itemObject)
{
    setData(QVariant::fromValue(reinterpret_cast<quint64>(itemObject)), DMHelper::TreeItemData_Object);
    setVisualization();
}

int CampaignTreeItem::getCampaignItemRow() const
{
    CampaignObjectBase* campaignObject = getCampaignItemObject();
    return (campaignObject == nullptr) ? -1 : campaignObject->getRow();
}

void CampaignTreeItem::setCampaignItemRow(int itemRow)
{
    CampaignObjectBase* campaignObject = getCampaignItemObject();
    if(campaignObject)
        campaignObject->setRow(itemRow);
}

CampaignTreeItem* CampaignTreeItem::getChildById(const QUuid& itemId) const
{
    for(int i = 0; i < rowCount(); ++i)
    {
        CampaignTreeItem* item = dynamic_cast<CampaignTreeItem*>(child(i));
        if(item)
        {
            if(item->getCampaignItemId() == itemId)
                return item;

            CampaignTreeItem* childItem = item->getChildById(itemId);
            if(childItem)
                return childItem;
        }
    }

    return nullptr;
}

CampaignTreeItem* CampaignTreeItem::getChildCampaignItem(int childRow) const
{
    return dynamic_cast<CampaignTreeItem*>(child(childRow));
}

void CampaignTreeItem::setPublishing(bool publishing)
{
    QFont f = font();
    f.setBold(publishing);
    setFont(f);
    setForeground(QBrush(publishing ? Qt::red : Qt::black));

    _isPublishing = publishing;
    setVisualization();
}

void CampaignTreeItem::updateVisualization()
{
    setVisualization();
}

void CampaignTreeItem::setVisualization()
{
    CampaignObjectBase* object = getCampaignItemObject();
    if(!object)
        return;

    if(_isPublishing)
    {
        setIcon(QIcon(":/img/data/icon_publishon.png"));
        return;
    }

    setIcon(object->getIcon());

    // Set the check-box for
    if(object->getObjectType() == DMHelper::CampaignType_Combatant)
    {
        Characterv2* character = dynamic_cast<Characterv2*>(object);
        bool isPC = ((character) && (character->isInParty()));
        setCheckable(isPC);
        if(isPC)
            setCheckState(character->getBoolValue(QString("active")) ? Qt::Checked : Qt::Unchecked);
        else
            setData(QVariant(), Qt::CheckStateRole); // Needed to actively remove the checkbox on the entry
    }

    /*
    switch(object->getObjectType())
    {
        case DMHelper::CampaignType_Party:
            setIcon(QIcon(":/img/data/icon_contentparty.png"));
            break;
        case DMHelper::CampaignType_Combatant:
            {
                Characterv2* character = dynamic_cast<Characterv2*>(object);
                bool isPC = ((character) && (character->isInParty()));
                setIcon(isPC ? QIcon(":/img/data/icon_contentcharacter.png") : QIcon(":/img/data/icon_contentnpc.png"));
                setCheckable(isPC);
                if(isPC)
                    setCheckState(character->getBoolValue(QString("active")) ? Qt::Checked : Qt::Unchecked);
                else
                    setData(QVariant(), Qt::CheckStateRole); // Needed to actively remove the checkbox on the entry
            }
            break;
        case DMHelper::CampaignType_Map:
            setIcon(QIcon(":/img/data/icon_contentmap.png"));
            break;
        case DMHelper::CampaignType_Text:
        case DMHelper::CampaignType_LinkedText:
            setIcon(QIcon(":/img/data/icon_contenttextencounter.png"));
            break;
        case DMHelper::CampaignType_Battle:
            setIcon(QIcon(":/img/data/icon_contentbattle.png"));
            break;
        case DMHelper::CampaignType_ScrollingText:
            setIcon(QIcon(":/img/data/icon_contentscrollingtext.png"));
            break;
        case DMHelper::CampaignType_AudioTrack:
            {
                QString audioIcon(":/img/data/icon_soundboard.png");
                AudioTrack* track = dynamic_cast<AudioTrack*>(object);
                if(track)
                {
                    if(track->getAudioType() == DMHelper::AudioType_Syrinscape)
                        audioIcon = QString(":/img/data/icon_syrinscape.png");
                    else if(track->getAudioType() == DMHelper::AudioType_Youtube)
                        audioIcon = QString(":/img/data/icon_playerswindow.png");
                }
                setIcon(QIcon(audioIcon));
            }
            break;
    }
    */
}
