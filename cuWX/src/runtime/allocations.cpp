#include <allocations.h>
#include <string.h>
#include <ioctl_wrapper.h>
#include <log.h>

Allocation allocations[MAX_ALLOCATIONS];

int allocation_count = 0;
int io_fd = -1;

Allocation *cudart_code_allocation = NULL;
Allocation *kernel_code_allocation = NULL;

AllocationsCallback callbacks[MAX_CALLBACKS];
int callback_count = 0;

bool add_allocation_callback(AllocationsCallback callback) {
    if (callback_count >= MAX_CALLBACKS) {
        return false;
    }
    callbacks[callback_count++] = callback;
    return true;
}

int find_allocation_id(NvU64 address) {
    for (int i = 0; i < allocation_count; i++) {
        Allocation *allocation = &allocations[i];
        if (allocation->base <= address && allocation-> base + allocation->length > address) {
            return i;
        }
    }
    return -1;
}

Allocation *find_allocation(NvU64 address) {
    int id = find_allocation_id(address);
    if (id == -1) {
        return NULL;
    }
    return &allocations[id];
}

Allocation *allocation_from_id(uint32_t id) {
    if (id >= allocation_count) {
        return NULL;
    }
    return &allocations[id];
}

bool add_allocation(UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *params) {
    
    if (allocation_count >= MAX_ALLOCATIONS) {
        return false;
    }

    Allocation *allocation = find_allocation(params->base);
    if (allocation == NULL) {
        allocation = &allocations[allocation_count++];
    }

    memcpy(allocation, params, sizeof(Allocation));

    if (allocation - allocations == CUDART_CODE_ALLOCATION_ID && cudart_code_allocation == NULL) {
        LOG_VERBOSE("Found cudart code allocation at index %d, base: 0x%016lx, length: 0x%016lx\n", CUDART_CODE_ALLOCATION_ID, allocation->base, allocation->length);
        cudart_code_allocation = allocation;
    }

    if (allocation - allocations == KERNEL_CODE_ALLOCATION_ID && kernel_code_allocation == NULL) {
        LOG_VERBOSE("Found kernel code allocation at index %d, base: 0x%016lx, length: 0x%016lx\n", KERNEL_CODE_ALLOCATION_ID, allocation->base, allocation->length);
        kernel_code_allocation = allocation;
    }

    if (allocation_count > CONTEXT_INIT_ALLOCATION_MIN && callback_count > 0) {
        LOG_VERBOSE("Code pages have been allocated, notifying callbacks...\n");

        for (int i = 0; i < callback_count; i++) {
            callbacks[i](allocation, allocation_count);
        }
        callback_count = 0; // Reset callbacks after notifying
    }

    return true;
}

bool add_free(UVM_FREE_PARAMS *params) {
    int id = find_allocation_id(params->base);
    if (id == -1) {
        return false;
    }

    memcpy(&allocations[id], &allocations[--allocation_count], sizeof(Allocation));
    return true;
}

void allocations_process_ioctl(int fd, uint8_t nr, size_t size, void *arg) {
    if (io_fd == -1 && nr == UVM_MAP_EXTERNAL_ALLOCATION) {
        io_fd = fd;
    }

    switch (nr) {
        case UVM_MAP_EXTERNAL_ALLOCATION: {
            UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *params = (UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *)arg;
            LOG_VERBOSE("ALLOC: 0x%016lx (len: 0x%016lx), mappingType: 0x%08x. Status: %d (0x%08x)\n", params->base, params->length, params->perGpuAttributes[0].gpuMappingType, params->rmStatus, params->rmStatus);
            add_allocation(params);
            break;
        }
        case UVM_FREE: {
            UVM_FREE_PARAMS *params = (UVM_FREE_PARAMS *)arg;
            LOG_VERBOSE("FREE:  0x%016lx (len: 0x%016lx).                          Status: %d (0x%08x)\n", params->base, params->length, params->rmStatus, params->rmStatus);
            add_free(params);
            break;
        }
    }
}

bool allocation_set_mapping_type(Allocation *allocation, UvmGpuMappingType mapping) {
    Allocation allocation_copy;
    memcpy(&allocation_copy, allocation, sizeof(Allocation));

    for(int i = 0; i < allocation_copy.gpuAttributesCount; i++) {
        allocation_copy.perGpuAttributes[i].gpuMappingType = mapping;
    }

    UVM_FREE_PARAMS free_params;
    free_params.base = allocation_copy.base;
    free_params.length = allocation_copy.length;

    LOG_VERBOSE("Freeing allocation (base: 0x%016lx, len: 0x%016lx)\n", allocation_copy.base, allocation_copy.length);
    int res = real_ioctl(io_fd, UVM_FREE, &free_params);
    if (res == -1 || free_params.rmStatus != NV_OK) {
        LOG("Failed to free allocation, res: %d, status: %x, err: %s\n", res, free_params.rmStatus, strerror(res));
        return false;
    }

    LOG_VERBOSE("Creating VA range...\n");
    UVM_CREATE_EXTERNAL_RANGE_PARAMS create_params;
    create_params.base = allocation_copy.base;
    create_params.length = allocation_copy.length;
    res = real_ioctl(io_fd, UVM_CREATE_EXTERNAL_RANGE, &create_params);
    if (res == -1 || create_params.rmStatus != NV_OK) {
        LOG("Failed to create VA range, res: %d, status: %x, err: %s\n", res, create_params.rmStatus, strerror(res));
        return false;
    }

    LOG_VERBOSE("Mapping allocation (base: 0x%016lx, len: 0x%016lx) with mapping type 0x%08x\n", allocation_copy.base, allocation_copy.length, mapping);
    res = real_ioctl(io_fd, UVM_MAP_EXTERNAL_ALLOCATION, &allocation_copy);
    if (res == -1 || allocation_copy.rmStatus != NV_OK) {
        LOG("Failed to map allocation, res: %d, status: %x, err: %s\n", res, allocation_copy.rmStatus, strerror(res));
        return false;
    }

    return true;
}

void print_allocations() {
    LOG("Allocations:\n");
    for (int i = 0; i < allocation_count; i++) {
        Allocation *allocation = &allocations[i];
        LOG("ALLOC: 0x%016lx (len: 0x%016lx), mappingType: 0x%08x\n", allocation->base, allocation->length, allocation->perGpuAttributes[0].gpuMappingType);
    }
}

bool set_ro_permissions(bool ro) {
#ifdef SET_RO_CODE
    LOG("Setting RO permissions: %s\n", ro ? "true" : "false");
    // NOTE: This code is not thread-safe, it should be protected by a mutex or similar mechanism.

    if (cudart_code_allocation) {
        LOG_VERBOSE("Setting RO permissions on cudart code allocation (base: 0x%016lx, length: 0x%016lx)\n", cudart_code_allocation->base, cudart_code_allocation->length);
        allocation_set_mapping_type(cudart_code_allocation, ro ? UvmGpuMappingTypeReadOnly : UvmGpuMappingTypeReadWrite);
    }
    if (kernel_code_allocation) {
        LOG_VERBOSE("Setting RO permissions on kernel code allocation (base: 0x%016lx, length: 0x%016lx)\n", kernel_code_allocation->base, kernel_code_allocation->length);
        allocation_set_mapping_type(kernel_code_allocation, ro ? UvmGpuMappingTypeReadOnly : UvmGpuMappingTypeReadWrite);
    }
#endif
    return true;
}

const Allocation *get_cudart_code_allocation() {
    return cudart_code_allocation;
}

const Allocation *get_kernel_code_allocation() {
    return kernel_code_allocation;
}
