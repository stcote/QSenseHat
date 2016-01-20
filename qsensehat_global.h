#ifndef QSENSEHAT_GLOBAL_H
#define QSENSEHAT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(QSENSEHAT_LIBRARY)
#  define QSENSEHATSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QSENSEHATSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QSENSEHAT_GLOBAL_H
