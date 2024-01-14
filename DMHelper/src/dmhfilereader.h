#ifndef DMHFILEREADER_H
#define DMHFILEREADER_H

#include <QObject>
#include <QMessageBox>

class DMHFileReader : public QObject
{
    Q_OBJECT
public:
    enum DMHFileError
    {
        DMHFileError_None = 0,
        DMHFileError_Empty,
        DMHFileError_NotFound,
        DMHFileError_Unsupported,
        DMHFileError_ReadError
    };

    explicit DMHFileReader(const QString& filename, const QString& objectName = QString(), bool accept = false, QObject *parent = nullptr);

    bool isAcceptErrors() const;
    DMHFileError getError() const;

    bool isImage();
    QImage loadImage();

    QString getFilename();

signals:

public slots:
    void setAccept(bool accept);

protected:
    bool isFile(const QString& typeName);
    void userMessage(const QString& message);
    QMessageBox::StandardButton userQuery(const QString& query);

    QString _filename;
    QString _objectName;
    bool _acceptErrors;
    DMHFileError _error;

};

#endif // DMHFILEREADER_H
