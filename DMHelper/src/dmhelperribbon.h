#ifndef DMHELPERRIBBON_H
#define DMHELPERRIBBON_H

#include <QTabWidget>

class DMHelperRibbon : public QTabWidget
{
    Q_OBJECT
public:
    explicit DMHelperRibbon(QWidget *parent = nullptr);
     ~DMHelperRibbon();
};

#endif // DMHELPERRIBBON_H
