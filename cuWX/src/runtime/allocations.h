#ifndef ALLOCATIONS_H
#define ALLOCATIONS_H

#include <cuda.h>
#include <nvtypes.h>
#include <uvm_types.h>
#include <uvm_ioctl.h>
#include <stdbool.h>

#define MAX_ALLOCATIONS 1000
#define MAX_CALLBACKS 10

#define CUDART_CODE_ALLOCATION_ID 15
#define KERNEL_CODE_ALLOCATION_ID 17
#define CONTEXT_INIT_ALLOCATION_MIN 17

typedef UVM_MAP_EXTERNAL_ALLOCATION_PARAMS Allocation;
typedef void (*AllocationsCallback)(Allocation *, size_t);

bool add_allocation(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *params);
bool add_free(UVM_FREE_PARAMS *params);
Allocation *find_allocation(NvU64 address);
void allocations_process_ioctl(int fd, uint8_t nr, size_t size, void *arg);
bool allocation_set_mapping_type(Allocation *allocation, UvmGpuMappingType mapping);
void print_allocations();
Allocation *allocation_from_id(uint32_t id);

/**
 * * Register a callback to be called when context initialization is done.
 * * This is useful for tools that need to perform actions after code pages allocation.
 * * @param callback The callback function to register.
 * * @return true if the callback was successfully registered, false otherwise.
 */
bool add_allocation_callback(AllocationsCallback callback);

/**
 * * Set read-only permissions on the cudart code allocation and the kernel code allocation.
 * * This is necessary to ensure that the code can be executed but not modified.
 * * @param ro If true, set read-only permissions; if false, unset them.
 * * @return true if the permissions were successfully set, false otherwise.
 */
bool set_ro_permissions(bool ro);

const Allocation *get_cudart_code_allocation();
const Allocation *get_kernel_code_allocation();

#endif
