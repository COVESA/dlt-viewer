#ifndef EXPORT_RULES_H
#define EXPORT_RULES_H

/// \file export_rules.h
/// \brief Export rules for C++ code to control symbols visibility
/// \details This file is used to control the visibility of symbols in the shared library. See
/// https://doc.qt.io/qt-5/sharedlibrary.html#using-symbols-from-shared-libraries for details

#include <QtCore/QtGlobal>

#if defined(QDLT_LIBRARY)
# define QDLT_EXPORT Q_DECL_EXPORT
#else
# define QDLT_EXPORT Q_DECL_IMPORT
#endif

#endif // EXPORT_RULES_H
