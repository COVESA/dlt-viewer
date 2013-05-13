#include <QtGlobal>

#ifndef EXPORT_RULES_H
#define EXPORT_RULES_H

#if defined(QDLT_LIBRARY)
# define QDLT_EXPORT Q_DECL_EXPORT
# ifdef Q_OS_WIN
#  define QDLT_C_EXPORT __declspec(dllexport)
# else
#  define QDLT_C_EXPORT __attribute__((visibility("default")))
# endif
#else
# define QDLT_EXPORT Q_DECL_IMPORT
# ifdef Q_OS_WIN
#  define QDLT_C_EXPORT __declspec(dllimport)
# else
#  define QDLT_C_EXPORT
# endif
#endif

#endif // EXPORT_RULES_H
