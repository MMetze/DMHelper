#include "optionsaccessor.h"

OptionsAccessor::OptionsAccessor(QObject *parent) :
    QSettings("Glacial Software", "DMHelper", parent)
{
#ifdef QT_DEBUG
    beginGroup("DEBUG");
#endif
}

OptionsAccessor::~OptionsAccessor()
{
#ifdef QT_DEBUG
    endGroup(); // DEBUG
#endif
}
