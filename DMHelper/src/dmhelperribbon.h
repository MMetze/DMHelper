#ifndef DMHELPERRIBBON_H
#define DMHELPERRIBBON_H

#include <QTabWidget>

class DMHelperRibbon : public QTabWidget
{
    Q_OBJECT
public:
    explicit DMHelperRibbon(QWidget *parent = nullptr);
    virtual ~DMHelperRibbon() override;

    void enableTab(QWidget* page);
    void disableTab(QWidget* page);
};

#endif // DMHELPERRIBBON_H
