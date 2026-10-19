#ifndef HEXDUMP_H
#define HEXDUMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

void hexdump(const void* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif