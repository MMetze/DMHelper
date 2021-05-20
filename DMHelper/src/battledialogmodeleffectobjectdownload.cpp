#include "battledialogmodeleffectobjectdownload.h"
#include "dmconstants.h"
#include <QPixmap>
#include <QDomElement>

BattleDialogModelEffectObjectDownload::BattleDialogModelEffectObjectDownload(const QString& name, QObject *parent) :
    BattleDialogModelEffectObject(name, parent),
    _token(),
    _md5(),
    _uuid()
{
}

BattleDialogModelEffectObjectDownload::BattleDialogModelEffectObjectDownload(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip) :
    BattleDialogModelEffectObject(size, width, position, rotation, imageFile, tip),
    _token(),
    _md5(),
    _uuid()
{
}

BattleDialogModelEffectObjectDownload::~BattleDialogModelEffectObjectDownload()
{
}

void BattleDialogModelEffectObjectDownload::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _token = element.attribute("token");
    interpretToken();

    BattleDialogModelEffectObject::inputXML(element, true);

    if(!getMD5().isEmpty())
        emit requestFile(getMD5(), getUuid(), DMHelper::FileType_Image);
}

QString BattleDialogModelEffectObjectDownload::getMD5() const
{
    return _md5;
}

void BattleDialogModelEffectObjectDownload::setMD5(const QString& md5)
{
    _md5 = md5;
}

QString BattleDialogModelEffectObjectDownload::getUuid() const
{
    return _uuid;
}

void BattleDialogModelEffectObjectDownload::setUuid(const QString& uuid)
{
    _uuid = uuid;
}

void BattleDialogModelEffectObjectDownload::interpretToken()
{
    QStringList splits = _token.split(QString(","));
    setMD5(splits.count() > 0 ? splits.at(0) : QString());
    setUuid(splits.count() > 1 ? splits.at(1) : QString());
}
