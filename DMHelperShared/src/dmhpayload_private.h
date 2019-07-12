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
    QString getCommand() const;
    void setCommand(const QString& command);
    QString getPayload() const;
    void setPayload(const QString& payload);

    void fromString(const QString& data);
    QString toString() const;

private:
    QString _imageFile;
    QString _audioFile;
    QString _command;
    QString _payload;

};

#endif // DMHPAYLOAD_PRIVATE_H
