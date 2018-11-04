#ifndef DMHGLOBAL_H
#define DMHGLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DMHSHARED_LIB)
#  define DMHSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DMHSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DMHGLOBAL_H
