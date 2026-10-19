#ifndef NV_ESC_IOCTL_H
#define NV_ESC_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <nv-ioctl.h>

int print_ctl_call(uint8_t nr, size_t size, void *arg);

#ifdef __cplusplus
}
#endif

#endif