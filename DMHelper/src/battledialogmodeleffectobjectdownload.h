#ifndef BATTLEDIALOGMODELEFFECTOBJECTDOWNLOAD_H
#define BATTLEDIALOGMODELEFFECTOBJECTDOWNLOAD_H

#include "battledialogmodeleffectobject.h"

class BattleDialogModelEffectObjectDownload : public BattleDialogModelEffectObject
{
    Q_OBJECT

public:
    BattleDialogModelEffectObjectDownload(const QString& name = QString(), QObject *parent = nullptr);
    explicit BattleDialogModelEffectObjectDownload(int size, int width, const QPointF& position, qreal rotation, const QString& imageFile, const QString& tip);
    virtual ~BattleDialogModelEffectObjectDownload() override;

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    virtual QString getMD5() const override;
    virtual void setMD5(const QString& md5) override;

    QString getUuid() const;
    void setUuid(const QString& uuid);

signals:
    void requestFile(const QString& md5, const QString& uuid, int fileType);

protected:
    void interpretToken();

    QString _token;
    QString _md5;
    QString _uuid;
};

#endif // BATTLEDIALOGMODELEFFECTOBJECTDOWNLOAD_H
