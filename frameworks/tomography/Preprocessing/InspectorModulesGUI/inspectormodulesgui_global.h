//<LICENSE>

#ifndef INSPECTORMODULESGUI_GLOBAL_H
#define INSPECTORMODULESGUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(INSPECTORMODULESGUI_LIBRARY)
#  define INSPECTORMODULESGUISHARED_EXPORT Q_DECL_EXPORT
#else
#  define INSPECTORMODULESGUISHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef __GNUC__
#define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_ ## x
#endif

#endif // INSPECTORMODULESGUI_GLOBAL_H
