#include "quickref.h"
#include "dmversion.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

QuickRef* QuickRef::_instance = nullptr;

QuickRef::QuickRef(QObject *parent) :
    QObject(parent),
    _quickRefData(),
    _majorVersion(1),
    _minorVersion(0)
{
}

QuickRef::~QuickRef()
{
    qDeleteAll(_quickRefData);
}

QuickRef* QuickRef::Instance()
{
    return _instance;
}

void QuickRef::Initialize()
{
    if(_instance)
        return;

    qDebug() << "[QuickRef] Initializing QuickRef Data";
    _instance = new QuickRef();
}

void QuickRef::Shutdown()
{
    delete _instance;
    _instance = nullptr;
}

QString QuickRef::getVersion() const
{
    return QString::number(_majorVersion) + "." + QString::number(_minorVersion);
}

int QuickRef::getMajorVersion() const
{
    return _majorVersion;
}

int QuickRef::getMinorVersion() const
{
    return _minorVersion;
}

bool QuickRef::isVersionCompatible() const
{
    return (_majorVersion == DMHelper::QUICKREF_MAJOR_VERSION);
}

bool QuickRef::isVersionIdentical() const
{
    return ((_majorVersion == DMHelper::QUICKREF_MAJOR_VERSION) && (_minorVersion == DMHelper::QUICKREF_MINOR_VERSION));
}

QString QuickRef::getExpectedVersion()
{
    return QString::number(DMHelper::QUICKREF_MAJOR_VERSION) + "." + QString::number(DMHelper::QUICKREF_MINOR_VERSION);
}

bool QuickRef::exists(const QString& sectionName) const
{
    return _quickRefData.contains(sectionName);
}

int QuickRef::count() const
{
    return _quickRefData.count();
}

QStringList QuickRef::getSectionList() const
{
    return _quickRefData.keys();
}

QuickRefSection* QuickRef::getSection(const QString& sectionName)
{
    return _quickRefData.value(sectionName, nullptr);
}

QuickRefSubsection* QuickRef::getSubsection(const QString& sectionName, int subSectionIndex)
{
    QuickRefSection* section = getSection(sectionName);
    if(!section)
        return nullptr;

    return section->getSubsection(subSectionIndex);
}

QuickRefData* QuickRef::getData(const QString& sectionName, int subSectionIndex, const QString& dataTitle)
{
    QuickRefSubsection* subSection = getSubsection(sectionName, subSectionIndex);
    if(!subSection)
        return nullptr;

    return subSection->getData(dataTitle);
}


void QuickRef::readQuickRef(const QString& quickRefFile)
{
    if(!_quickRefData.empty())
        return;

    QDomDocument doc("DMHelperDataXML");
    QFile file(quickRefFile);
    qDebug() << "[QuickRef] Quickref data file: " << QFileInfo(file).filePath();
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "[QuickRef] Unable to read quickref file: " << quickRefFile;
        return;
    }

    QFileInfo quickRefInfo(quickRefFile);
    QString quickRefIconDir = quickRefInfo.dir().absolutePath() + QString("/icons/");

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString errMsg;
    int errRow;
    int errColumn;
    bool contentResult = doc.setContent(in.readAll(), &errMsg, &errRow, &errColumn);

    file.close();

    if(contentResult == false)
    {
        qDebug() << "[QuickRef] Unable to parse the quickref data file.";
        qDebug() << errMsg << errRow << errColumn;
        return;
    }

    QDomElement root = doc.documentElement();
    if((root.isNull()) || (root.tagName() != "root"))
    {
        qDebug() << "[QuickRef] Unable to find the root element in the quickref data file.";
        return;
    }

    QDomElement sectionElement = root.firstChildElement(QString("section"));
    while(!sectionElement.isNull())
    {
        QuickRefSection* newSection = new QuickRefSection(sectionElement, quickRefIconDir);
        _quickRefData.insert(newSection->getName(), newSection);
        sectionElement = sectionElement.nextSiblingElement(QString("section"));
    }

    emit changed();
}

QuickRefSection::QuickRefSection(QDomElement &element, const QString& iconDir) :
    _name(element.firstChildElement(QString("name")).text()),
    _limitation(element.firstChildElement(QString("limitation")).text()),
    _subSections()
{
    QDomElement subSectionElement = element.firstChildElement(QString("subsection"));
    while(!subSectionElement.isNull())
    {
        _subSections.append(new QuickRefSubsection(subSectionElement, iconDir));
        subSectionElement = subSectionElement.nextSiblingElement(QString("subsection"));
    }
}

QuickRefSection::~QuickRefSection()
{
    qDeleteAll(_subSections);
}

QString QuickRefSection::getName() const
{
    return _name;
}

QString QuickRefSection::getLimitation() const
{
    return _limitation;
}

int QuickRefSection::count() const
{
    return _subSections.count();
}

QuickRefSubsection* QuickRefSection::getSubsection(int index) const
{
    if((index < 0) || (index >= count()))
        return nullptr;

    return _subSections.at(index);
}

QList<QuickRefSubsection*> QuickRefSection::getSubsections() const
{
    return _subSections;
}


QuickRefSubsection::QuickRefSubsection(QDomElement &element, const QString& iconDir) :
    _description(element.firstChildElement(QString("description")).text()),
    _data()
{
    QDomElement dataElement = element.firstChildElement(QString("data"));
    while(!dataElement.isNull())
    {
        QuickRefData* newData = new QuickRefData(dataElement, iconDir);
        _data.insert(newData->getTitle(), newData);
        dataElement = dataElement.nextSiblingElement(QString("data"));
    }
}

QuickRefSubsection::~QuickRefSubsection()
{
    qDeleteAll(_data);
}

QString QuickRefSubsection::getDescription() const
{
    return _description;
}

bool QuickRefSubsection::exists(const QString& dataName) const
{
    return _data.contains(dataName);
}

int QuickRefSubsection::count() const
{
    return _data.count();
}

QStringList QuickRefSubsection::getDataTitles() const
{
    return _data.keys();
}

QuickRefData* QuickRefSubsection::getData(const QString& dataTitle)
{
    return _data.value(dataTitle, nullptr);
}


QuickRefData::QuickRefData(QDomElement &element, const QString& iconDir) :
    _title(element.attribute("title")),
    _icon(),
    _subtitle(element.firstChildElement(QString("subtitle")).text()),
    _description(element.firstChildElement(QString("description")).text()),
    _reference(element.firstChildElement(QString("reference")).text()),
    _bullets()
{
    QString iconName = element.firstChildElement(QString("icon")).text();
    QString resourceIcon = QString(":/img/data/img/") + iconName + QString(".png");
    if(QFile::exists(resourceIcon))
    {
        _icon = resourceIcon;
    }
    else
    {
        QString fileIcon = iconDir + iconName + QString(".png");
        if(QFile::exists(fileIcon))
            _icon = fileIcon;
        else
            qDebug() << "[QuickRefData] ERROR: Unable to find icon '" << iconName << "' in path: " << fileIcon;
    }

    QDomElement bulletsElement = element.firstChildElement(QString("bullets"));
    if(!bulletsElement.isNull())
    {
        QDomElement bulletElement = bulletsElement.firstChildElement(QString("bullet"));
        while(!bulletElement.isNull())
        {
            QDomCDATASection bulletData = bulletElement.firstChild().toCDATASection();
            _bullets.append(bulletData.data());
            bulletElement = bulletElement.nextSiblingElement(QString("bullet"));
        }
    }
}

QuickRefData::~QuickRefData()
{
}

QString QuickRefData::getTitle() const
{
    return _title;
}

QString QuickRefData::getIcon() const
{
    return _icon;
}

QString QuickRefData::getSubtitle() const
{
    return _subtitle;
}

QString QuickRefData::getDescription() const
{
    return _description;
}

QString QuickRefData::getReference() const
{
    return _reference;
}

QStringList QuickRefData::getBullets() const
{
    return _bullets;
}

QString QuickRefData::getOverview() const
{
    QString result;

    if(!_description.isEmpty())
        result += QString("<i>") + _description + QString("</i>") + QString("<p>");

    if(_bullets.count() > 0)
    {
        result += QString("<ul>");
        for(const QString& bulletLine : std::as_const(_bullets))
            result += QString("<li>") + bulletLine + QString("</li>");
        result += QString("</ul>");
    }

    if(!_reference.isEmpty())
        result += QString("<p>") + QString("<i>") + _reference + QString("</i>");

    return result;
}
