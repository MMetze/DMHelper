#ifndef ENCOUNTERTEXTLINKED_H
#define ENCOUNTERTEXTLINKED_H

#include "encountertext.h"

class QFileSystemWatcher;

class EncounterTextLinked : public EncounterText
{
    Q_OBJECT
public:
    explicit EncounterTextLinked(const QString& encounterName = QString(), QObject *parent = nullptr);

    // From CampaignObjectBase
    virtual void inputXML(const QDomElement &element, bool isImport) override;
    virtual void copyValues(const CampaignObjectBase* other) override;
    virtual int getObjectType() const override;

    // Linked File
    virtual QString getLinkedFile() const;
    virtual int getFileType() const;

    // Metadata
    virtual QString getMetadata() const;

public slots:
    // Text
    virtual void setText(const QString& newText) override;

    // Linked File
    virtual void setLinkedFile(const QString& filename);
    virtual void setWatcher(bool enable);

signals:

protected slots:
    virtual QDomElement createOutputXML(QDomDocument &doc) override;
    virtual void internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual void readLinkedFile();

protected:
    virtual void createTextNode(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) override;
    virtual void extractTextNode(const QDomElement &element, bool isImport) override;

    QString extractMetadata(const QString& inputString);
    void parseMetadata();

    QString _linkedFile;
    QFileSystemWatcher* _watcher;
    int _fileType;
    QString _metadata;
};

#endif // ENCOUNTERTEXTLINKED_H
