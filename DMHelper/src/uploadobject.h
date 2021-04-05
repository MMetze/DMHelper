#ifndef UPLOADOBJECT_H
#define UPLOADOBJECT_H

#include <QString>

class CampaignObjectBase;

class UploadObject
{
public:

    enum UploadObject_Status
    {
        Status_Complete = 0,
        Status_NotStarted = -1,
        Status_Error = -2,
        Status_Exists = -3
    };

    explicit UploadObject();
    explicit UploadObject(CampaignObjectBase* object);
    explicit UploadObject(CampaignObjectBase* object, int status);
    explicit UploadObject(CampaignObjectBase* object, const QString& filename, int status);
    virtual ~UploadObject();

    virtual bool isValid() const;
    virtual bool hasMD5() const;

    virtual CampaignObjectBase* getObject() const;
    virtual void setObject(CampaignObjectBase* object);

    virtual QString getFilename() const;
    virtual void setFilename(const QString& filename);

    virtual int getFileType() const;
    virtual void setFileType(int filetype);

    virtual QString getData() const;
    virtual void setData(const QString& data);

    virtual QString getMD5() const;
    virtual void setMD5(const QString& md5);

    virtual QString getDescription() const;
    virtual void setDescription(const QString& description);

    virtual int getStatus() const;
    virtual void setStatus(int status);

protected:
    CampaignObjectBase* _object;
    QString _filename;
    int _filetype;
    QString _data;
    QString _md5;
    QString _description;
    int _status;
};


#endif // UPLOADOBJECT_H
