#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifdef PROD
    #define LOG(...)
    #define LOG_VERBOSE(...)
#else
    #define LOG(...) fprintf(stderr, "[cuDEP]: "); fprintf(stderr, __VA_ARGS__)
    #ifdef VERBOSE
        #define LOG_VERBOSE(...) LOG(__VA_ARGS__)
    #else
        #define LOG_VERBOSE(...)
    #endif
#endif

#define LOG_ERROR(...) fprintf(stderr, "[cuDEP]: ERROR: "); fprintf(stderr, __VA_ARGS__)
#define LOG_WARNING(...) fprintf(stderr, "[cuDEP]: WARNING: "); fprintf(stderr, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif