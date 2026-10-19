#ifndef ALLOCATIONS_H
#define ALLOCATIONS_H

#include <cuda.h>
#include <nvtypes.h>
#include <uvm_types.h>
#include <uvm_ioctl.h>
#include <stdbool.h>

#define MAX_ALLOCATIONS 1000
#define MAX_CALLBACKS 10

typedef UVM_MAP_EXTERNAL_ALLOCATION_PARAMS Allocation;
typedef void (*AllocationsCallback)(Allocation *, size_t);

/**
 * Find the allocation that contains the given address and return its index in the allocations array.
 * @param address The address to find the allocation for.
 * @return The index of the allocation that contains the given address, or -1 if no allocation contains the address.
 */
int find_allocation_id(NvU64 address);

bool add_allocation(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *params);
bool add_free(UVM_FREE_PARAMS *params);
Allocation *find_allocation(NvU64 address);
void allocations_process_ioctl(int fd, uint8_t nr, size_t size, void *arg);
void print_allocations();
Allocation *allocation_from_id(uint32_t id);

#endif