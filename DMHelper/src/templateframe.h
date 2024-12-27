#ifndef TEMPLATEFRAME_H
#define TEMPLATEFRAME_H

class QObject;
class QWidget;
class QString;
class TemplateObject;

class TemplateFrame
{
public:
    TemplateFrame();
    virtual ~TemplateFrame();

    virtual void handleEditBoxChange(QWidget* editWidget, TemplateObject* templateObject, const QString& value);
    virtual void handleAddResource(QWidget* widget, TemplateObject* templateObject);
    virtual void handleRemoveResource(QWidget* widget, TemplateObject* templateObject);

protected:
    virtual QObject* getFrameObject() = 0;
};

#endif // TEMPLATEFRAME_H
