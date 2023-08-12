#include "dmhfilereader.h"
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QImageReader>
#include <QDebug>

DMHFileReader::DMHFileReader(const QString& filename, const QString& objectName, bool accept, QObject *parent) :
    QObject{parent},
    _filename(filename),
    _objectName(objectName),
    _acceptErrors(accept),
    _error(DMHFileError_None)
{
}

bool DMHFileReader::isAcceptErrors() const
{
    return _acceptErrors;
}

DMHFileReader::DMHFileError DMHFileReader::getError() const
{
    return _error;
}

bool DMHFileReader::isImage()
{
    if(!isFile(QString("Image")))
        return false;

    if(QImageReader::imageFormat(_filename).isEmpty())
    {
        _error = DMHFileError_Unsupported;
        userMessage(QString("The image file ") + _filename + QString(" is of an unknown or unsupported format."));
        return false;
    }

    _error = DMHFileError_None;
    return true;
}

QImage DMHFileReader::loadImage()
{
    if(!isImage())
        return QImage();

    QImageReader reader(_filename);
    QImage image = reader.read();
    if(image.isNull())
    {
        _error = DMHFileError_Unsupported;
        userMessage(QString("Not able to read image file ") + _filename + QString(": ") + QString::number(reader.error()) + QString(", ") + reader.errorString() + QString("\n Image Format: ") + QString::number(reader.imageFormat()));
        return QImage();
    }

    if(image.format() != QImage::Format_ARGB32_Premultiplied)
        image.convertTo(QImage::Format_ARGB32_Premultiplied);

    return image;
}

QString DMHFileReader::getFilename()
{
    return _filename;
}

void DMHFileReader::setAccept(bool accept)
{
    _acceptErrors = accept;
}

bool DMHFileReader::isFile(const QString& typeName)
{
    if(_filename.isEmpty())
    {
        _error = DMHFileError_Empty;
        userMessage(QString("The ") + typeName + QString(" file to be read is empty!"));
        return false;
    }

    if(!QFile::exists(_filename))
    {
        QMessageBox::StandardButton result = userQuery(QString("The ") + typeName + QString(" file could not be found: ") + _filename + QString("\n Do you want to select a new ") + typeName + QString(" file?"));
        if(result == QMessageBox::Yes)
        {
            QString newFileName = QFileDialog::getOpenFileName(nullptr, QString("DMHelper New ") + typeName + QString(" File"));
            if(!newFileName.isEmpty())
            {
                _filename = newFileName;
                return isFile(typeName);
            }
        }

        _error = DMHFileError_NotFound;
        return false;
    }

    return true;
}

void DMHFileReader::userMessage(const QString& message)
{
    QString localMessage = (_objectName.isEmpty()) ? QString() : (QString("For ") + _objectName + QString(": "));
    localMessage += message;

    qDebug() << "[DMHFileReader] ERROR: " << message << "(accept: " << _acceptErrors << ")";

    if(_acceptErrors)
        return;

    QMessageBox::critical(nullptr, QString("DMHelper File Read Error"), message);
}

QMessageBox::StandardButton DMHFileReader::userQuery(const QString& query)
{
    QString localQuery = (_objectName.isEmpty()) ? QString() : (QString("For ") + _objectName + QString(": "));
    localQuery += query;

    if(_acceptErrors)
    {
        qDebug() << "[DMHFileReader] ERROR QUERY (AUTO-REJECTED): " << localQuery;
        return QMessageBox::No;
    }

    QMessageBox::StandardButton result = QMessageBox::critical(nullptr, QString("DMHelper File Read Error"), localQuery, QMessageBox::Yes | QMessageBox::No | QMessageBox::NoAll, QMessageBox::Yes);
    qDebug() << "[DMHFileReader] ERROR QUERY: " << localQuery << ", RESULT: " << result;
    if(result == QMessageBox::NoAll)
        _acceptErrors = true;
    return result;
}
