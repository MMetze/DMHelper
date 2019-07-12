#ifndef MRUACTION_H
#define MRUACTION_H

#include <QAction>

class MRUAction : public QAction
{
    Q_OBJECT
public:
    explicit MRUAction (const QString& targetFile, QObject *parent = nullptr);
    explicit MRUAction (const QString& targetFile, const QString & text, QObject * parent = nullptr);
    explicit MRUAction (const QString& targetFile, const QIcon & icon, const QString & text, QObject * parent = nullptr);

signals:
    void triggerFile(const QString& targetFile);

private slots:
    void basicTrigger();

private:
    QString mTargetFile;

};

#endif // MRUACTION_H
