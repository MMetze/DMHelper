#include "mruaction.h"

MRUAction::MRUAction( const QString& targetFile, QObject *parent ) :
    QAction(parent),
    mTargetFile(targetFile)
{
    connect(this,SIGNAL(triggered()),this,SLOT(basicTrigger()));
}

MRUAction::MRUAction( const QString& targetFile, const QString & text, QObject * parent ) :
    QAction(text, parent),
    mTargetFile(targetFile)
{
    connect(this,SIGNAL(triggered()),this,SLOT(basicTrigger()));
}

MRUAction::MRUAction( const QString& targetFile, const QIcon & icon, const QString & text, QObject * parent ) :
    QAction(icon, text, parent),
    mTargetFile(targetFile)
{
    connect(this,SIGNAL(triggered()),this,SLOT(basicTrigger()));
}

void MRUAction::basicTrigger()
{
    emit triggerFile(mTargetFile);
}
