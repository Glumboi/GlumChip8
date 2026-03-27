#ifndef HELPER_DEFINES_H
#define HELPER_DEFINES_H
#ifdef _WIN32
#define EXPORT_FN __declspec(dllexport)
#else
#define EXPORT_FN
#endif
#endif