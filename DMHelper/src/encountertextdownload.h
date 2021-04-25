#ifndef ENCOUNTERTEXTDOWNLOAD_H
#define ENCOUNTERTEXTDOWNLOAD_H

#include "encountertext.h"

class EncounterTextDownload : public EncounterText
{
    Q_OBJECT
public:
    explicit EncounterTextDownload(const QString& cacheDirectory, QObject *parent = nullptr);

    virtual void inputXML(const QDomElement &element, bool isImport) override;

    bool isComplete();

public slots:
    virtual void fileReceived(const QString& md5, const QString& uuid, const QByteArray& data);

signals:
    void requestFile(const QString& md5, const QString& uuid, int fileType);
    void encounterComplete();

protected:
    QString _md5Text;
    QString _md5TranslatedText;
    QString _cacheDirectory;

};

#endif // ENCOUNTERTEXTDOWNLOAD_H
