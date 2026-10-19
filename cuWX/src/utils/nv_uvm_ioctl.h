#ifndef NV_UVM_IOCTL_H
#define NV_UVM_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <uvm_ioctl.h>
#include <stdio.h>

int print_uvm_call(uint8_t nr, size_t size, void *arg);
// int log_allocations(uint8_t nr, size_t size, void *arg);

#ifdef __cplusplus
}
#endif

#endif