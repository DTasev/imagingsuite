#ifndef PROCESSFRAMEWORK_GLOBAL_H
#define PROCESSFRAMEWORK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PROCESSFRAMEWORK_LIBRARY)
#  define PROCESSFRAMEWORKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PROCESSFRAMEWORKSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PROCESSFRAMEWORK_GLOBAL_H
