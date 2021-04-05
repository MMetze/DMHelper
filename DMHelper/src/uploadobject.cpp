#include "uploadobject.h"
#include "campaignobjectbase.h"
#include "dmconstants.h"

UploadObject::UploadObject() :
    _object(nullptr),
    _filename(),
    _filetype(DMHelper::FileType_Other),
    _data(),
    _md5(),
    _description(),
    _status(Status_NotStarted)
{
}

UploadObject::UploadObject(CampaignObjectBase* object) :
    _object(object),
    _filename(),
    _filetype(DMHelper::FileType_Other),
    _data(),
    _md5(),
    _description(),
    _status(Status_NotStarted)
{
}

UploadObject::UploadObject(CampaignObjectBase* object, int status) :
    _object(object),
    _filename(),
    _filetype(DMHelper::FileType_Other),
    _data(),
    _md5(),
    _description(),
    _status(status)
{
}

UploadObject::UploadObject(CampaignObjectBase* object, const QString& filename, int status) :
    _object(object),
    _filename(filename),
    _filetype(DMHelper::FileType_Other),
    _data(),
    _md5(),
    _description(),
    _status(status)
{
}

UploadObject::~UploadObject()
{
}

bool UploadObject::isValid() const
{
    return((_object != nullptr) || (!_data.isEmpty()));
}

bool UploadObject::hasMD5() const
{
    return((!_md5.isEmpty()) ||
           ((_object != nullptr) && (!_object->getMD5().isEmpty())));
}

CampaignObjectBase* UploadObject::getObject() const
{
    return _object;
}

void UploadObject::setObject(CampaignObjectBase* object)
{
    _object = object;
}

QString UploadObject::getFilename() const
{
    return _filename;
}

void UploadObject::setFilename(const QString& filename)
{
    _filename = filename;
}

int UploadObject::getFileType() const
{
    return _filetype;
}

void UploadObject::setFileType(int filetype)
{
    _filetype = filetype;
}

QString UploadObject::getData() const
{
    return _data;
}

void UploadObject::setData(const QString& data)
{
    _data = data;
}

QString UploadObject::getMD5() const
{
    if(_object)
        return _object->getMD5();
    else
        return _md5;
}

void UploadObject::setMD5(const QString& md5)
{
    if(_object)
        _object->setMD5(md5);
    else
        _md5 = md5;
}

QString UploadObject::getDescription() const
{
    return _description;
}

void UploadObject::setDescription(const QString& description)
{
    _description = description;
}

int UploadObject::getStatus() const
{
    return _status;
}

void UploadObject::setStatus(int status)
{
    _status = status;
}
