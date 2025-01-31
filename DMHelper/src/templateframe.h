#ifndef TEMPLATEFRAME_H
#define TEMPLATEFRAME_H

#include <QString>

class QObject;
class QWidget;
class TemplateObject;

class TemplateFrame
{
public:
    TemplateFrame();
    virtual ~TemplateFrame();

    virtual void handleEditBoxChange(QWidget* editWidget, TemplateObject* templateObject, const QString& value);
    virtual void handleAddResource(QWidget* widget, TemplateObject* templateObject);
    virtual void handleRemoveResource(QWidget* widget, TemplateObject* templateObject);

    virtual QString getUIFilename() const;

protected:
    virtual QObject* getFrameObject() = 0;

    QString _uiFilename;
};

#endif // TEMPLATEFRAME_H
