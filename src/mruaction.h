#ifndef MRUACTION_H
#define MRUACTION_H

#include <QAction>

class MRUAction : public QAction
{
    Q_OBJECT
public:
    explicit MRUAction ( const QString& targetFile, QObject *parent = 0 );
    explicit MRUAction ( const QString& targetFile, const QString & text, QObject * parent = 0 );
    explicit MRUAction ( const QString& targetFile, const QIcon & icon, const QString & text, QObject * parent = 0 );

signals:
    void triggerFile(const QString& targetFile);

private slots:
    void basicTrigger();

private:
    QString mTargetFile;

};

#endif // MRUACTION_H
