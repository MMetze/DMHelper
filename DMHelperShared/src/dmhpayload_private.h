#ifndef DMHPAYLOAD_PRIVATE_H
#define DMHPAYLOAD_PRIVATE_H

#include <QString>

class DMHPayload_Private
{
public:
    DMHPayload_Private();
    explicit DMHPayload_Private(const QString& data);

    QString getImageFile() const;
    void setImageFile(const QString& imageFile);
    QString getAudioFile() const;
    void setAudioFile(const QString& audioFile);
    QString getData() const;
    void setData(const QString& data);

    void fromString(const QString& data);
    QString toString() const;

private:
    QString _imageFile;
    QString _audioFile;
    QString _data;

};

#endif // DMHPAYLOAD_PRIVATE_H
