#ifndef MONSTERCLASSV2_H
#define MONSTERCLASSV2_H

#include "templateobject.h"
#include <QObject>

class QDomElement;

class MonsterClassv2 : public QObject, public TemplateObject
{
    Q_OBJECT
public:
    explicit MonsterClassv2(const QString& name, QObject *parent = nullptr);
    explicit MonsterClassv2(const QDomElement &element, bool isImport, QObject *parent = nullptr);

    void inputXML(const QDomElement &element, bool isImport);
    QDomElement outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport) const;

    void beginBatchChanges();
    void endBatchChanges();

    int getType() const;
    bool getPrivate() const;

    int getIconCount() const;
    QStringList getIconList() const;
    QString getIcon(int index = 0) const;
    QPixmap getIconPixmap(DMHelper::PixmapSize iconSize, int index = 0);

    void cloneMonster(MonsterClassv2& other);

    static int convertSizeToCategory(const QString& monsterSize);
    static QString convertCategoryToSize(int category);
    static qreal convertSizeCategoryToScaleFactor(int category);
    static qreal convertSizeToScaleFactor(const QString& monsterSize);
    static void outputValue(QDomDocument &doc, QDomElement &element, bool isExport, const QString& valueName, const QString& valueText);

public slots:
    void setPrivate(bool isPrivate);
    void addIcon(const QString& iconFile);
    void setIcon(int index, const QString& iconFile);
    void removeIcon(int index);
    void removeIcon(const QString& iconFile);
    void searchForIcons();
    void refreshIconPixmaps();
    void clearIcon();

signals:
    void iconChanged(MonsterClassv2* monsterClass);
    void dirty();

protected:
    void registerChange();
    void readIcons(const QDomElement& element, bool isImport);
    void writeIcons(QDomDocument &doc, QDomElement& element, QDir& targetDirectory, bool isExport) const;

    bool _private;
    QStringList _icons;

    QHash<QString, QVariant> _allValues;

    bool _batchChanges;
    bool _changesMade;
    bool _iconChanged;

    QList<ScaledPixmap> _scaledPixmaps;
};

#endif // MONSTERCLASSV2_H
