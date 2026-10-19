#include <allocations.h>
#include <string.h>
#include <ioctl_wrapper.h>
#include <log.h>

Allocation allocations[MAX_ALLOCATIONS];

int allocation_count = 0;
int io_fd = -1;

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

void print_allocations() {
    LOG("Allocations:\n");
    for (int i = 0; i < allocation_count; i++) {
        Allocation *allocation = &allocations[i];
        LOG("ALLOC: 0x%016lx (len: 0x%016lx), mappingType: 0x%08x\n", allocation->base, allocation->length, allocation->perGpuAttributes[0].gpuMappingType);
    }
}