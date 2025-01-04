#ifndef EXPORT_C_RULES_H
#define EXPORT_C_RULES_H

/// \file export_c_rules.h
/// \brief Export rules for C code to control symbols visibility

#if defined(QDLT_LIBRARY)
# if defined(_WIN32) || defined(_WIN64)
#  define QDLT_C_EXPORT __declspec(dllexport)
# else
#  define QDLT_C_EXPORT __attribute__((visibility("default")))
# endif
#else
# if defined(_WIN32) || defined(_WIN64)
#  define QDLT_C_EXPORT __declspec(dllimport)
# else
#  define QDLT_C_EXPORT
# endif
#endif

#endif // EXPORT_C_RULES_H
