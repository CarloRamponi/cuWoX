#include <nv_esc_ioctl.h>
#include <nv_escape.h>
#include <log.h>
#include <nv-unix-nvos-params-wrappers.h>

int print_ctl_call(uint8_t nr, size_t size, void *arg) {
    switch (nr) {
        case NV_ESC_CARD_INFO: {
            LOG("NV_ESC_CARD_INFO\n");
            // kernel respects this size; see open-gpu-kernel-modules/kernel-open/nvidia/nv.c:2451
            const size_t max_card_infos = 128;
            nv_ioctl_card_info_t *card_infos = (nv_ioctl_card_info_t *)arg;
            for(int i = 0; i < max_card_infos && card_infos[i].valid; i++) {
                LOG_VERBOSE("card_infos[%d]:\n", i);
                LOG_VERBOSE("  pci_info:\n");
                LOG_VERBOSE("    domain:\t\t%08x\n", card_infos[i].pci_info.domain);
                LOG_VERBOSE("    bus:\t\t%02hhx\n", card_infos[i].pci_info.bus);
                LOG_VERBOSE("    slot:\t\t%02hhx\n", card_infos[i].pci_info.slot);
                LOG_VERBOSE("    function:\t\t%02hhx\n", card_infos[i].pci_info.function);
                LOG_VERBOSE("    vendor_id:\t\t%04hx\n", card_infos[i].pci_info.vendor_id);
                LOG_VERBOSE("    device_id:\t\t%04hx\n", card_infos[i].pci_info.device_id);
                LOG_VERBOSE("  gpu_id:\t\t%08x\n", card_infos[i].gpu_id);
                LOG_VERBOSE("  interrupt_line:\t%08x\n", card_infos[i].interrupt_line);
                LOG_VERBOSE("  reg_address:\t\t%016lx\n", card_infos[i].reg_address);
                LOG_VERBOSE("  reg_size:\t\t%016lx\n", card_infos[i].reg_size);
                LOG_VERBOSE("  fb_address:\t\t%016lx\n", card_infos[i].fb_address);
                LOG_VERBOSE("  fb_size:\t\t%016lx\n", card_infos[i].fb_size);
                LOG_VERBOSE("  minor_number:\t\t%08x\n", card_infos[i].minor_number);
            }
            return 1;
        }
        case NV_ESC_REGISTER_FD: {
            LOG("NV_ESC_REGISTER_FD\n");
            nv_ioctl_register_fd_t *params = (nv_ioctl_register_fd_t *)arg;
            LOG_VERBOSE("ctl_fd: %d\n", params->ctl_fd);
            return 1;
        }
        case NV_ESC_ALLOC_OS_EVENT: {
            LOG("NV_ESC_ALLOC_OS_EVENT\n");
            nv_ioctl_alloc_os_event_t *params = (nv_ioctl_alloc_os_event_t *)arg;
            LOG_VERBOSE("hClient: %d, hDevice: %d, fd: %d, Status: %d\n", params->hClient, params->hDevice, params->fd, params->Status);
            return 1;
        }
        case NV_ESC_FREE_OS_EVENT: {
            LOG("NV_ESC_FREE_OS_EVENT\n");
            nv_ioctl_free_os_event_t *params = (nv_ioctl_free_os_event_t *)arg;
            LOG_VERBOSE("hClient: %d, hDevice: %d, fd: %d, Status: %d\n", params->hClient, params->hDevice, params->fd, params->Status);
            return 1;
        }
        case NV_ESC_STATUS_CODE: {
            LOG("NV_ESC_STATUS_CODE\n");
            nv_ioctl_status_code_t *params = (nv_ioctl_status_code_t *)arg;
            LOG_VERBOSE("domain: %d, bus: %d, slot: %d, status: %d\n", params->domain, params->bus, params->slot, params->status);
            return 1;
        }
        case NV_ESC_CHECK_VERSION_STR: {
            LOG("NV_ESC_CHECK_VERSION_STR\n");
            nv_ioctl_rm_api_version_t *params = (nv_ioctl_rm_api_version_t *)arg;
            LOG_VERBOSE("cmd: %d, reply: %d, versionString: %s\n", params->cmd, params->reply, params->versionString);
            return 1;
        }
        case NV_ESC_IOCTL_XFER_CMD: {
            LOG("NV_ESC_IOCTL_XFER_CMD\n");
            nv_ioctl_xfer_t *params = (nv_ioctl_xfer_t *)arg;
            LOG_VERBOSE("cmd: %d, size: %d, ptr: %p\n", params->cmd, params->size, params->ptr);
            return 1;
        }
        case NV_ESC_RM_ALLOC_MEMORY: {
            LOG("NV_ESC_RM_ALLOC_MEMORY\n");
            nv_ioctl_nvos02_parameters_with_fd *params = (nv_ioctl_nvos02_parameters_with_fd *)arg;
            LOG_VERBOSE("pMemory: %p, limit: %p, flags: 0x%08x, status: 0x%08x\n", params->params.pMemory, params->params.limit, params->params.flags, params->params.status);
            return 1;
        }
        case NV_ESC_RM_ALLOC_OBJECT: {
            LOG("NV_ESC_RM_ALLOC_OBJECT\n");
            return 1;
        }
        case NV_ESC_RM_FREE: {
            LOG("NV_ESC_RM_FREE\n");
            return 1;
        }
        case NV_ESC_RM_CONTROL: {
            LOG("NV_ESC_RM_CONTROL\n");
            
            return 1;
        }
        case NV_ESC_RM_ALLOC: {
            LOG("NV_ESC_RM_ALLOC\n");
            if(size == sizeof(NVOS64_PARAMETERS)) {
                NVOS64_PARAMETERS *params = (NVOS64_PARAMETERS *)arg;
                LOG_VERBOSE("hObjectNew: 0x%08x, hClass: 0x%08x, flags: 0x%08x, status: 0x%08x\n", params->hObjectNew, params->hClass, params->flags, params->status);
            } else if(size == sizeof(NVOS21_PARAMETERS)) {
                NVOS21_PARAMETERS *params = (NVOS21_PARAMETERS *)arg;
                LOG_VERBOSE("hObjectNew: 0x%08x, hClass: 0x%08x, status: 0x%08x\n", params->hObjectNew, params->hClass, params->status);
            }
            return 1;
        }
        case NV_ESC_RM_CONFIG_GET: {
            LOG("NV_ESC_RM_CONFIG_GET\n");
            
            return 1;
        }
        case NV_ESC_RM_CONFIG_SET: {
            LOG("NV_ESC_RM_CONFIG_SET\n");
            
            return 1;
        }
        case NV_ESC_RM_DUP_OBJECT: {
            LOG("NV_ESC_RM_DUP_OBJECT\n");
            
            return 1;
        }
        case NV_ESC_RM_SHARE: {
            LOG("NV_ESC_RM_SHARE\n");
            
            return 1;
        }
        case NV_ESC_RM_CONFIG_GET_EX: {
            LOG("NV_ESC_RM_CONFIG_GET_EX\n");
            
            return 1;
        }
        case NV_ESC_RM_CONFIG_SET_EX: {
            LOG("NV_ESC_RM_CONFIG_SET_EX\n");
            
            return 1;
        }
        case NV_ESC_RM_I2C_ACCESS: {
            LOG("NV_ESC_RM_I2C_ACCESS\n");
            
            return 1;
        }
        case NV_ESC_RM_IDLE_CHANNELS: {
            LOG("NV_ESC_RM_IDLE_CHANNELS\n");
            
            return 1;
        }
        case NV_ESC_RM_VID_HEAP_CONTROL: {
            LOG("NV_ESC_RM_VID_HEAP_CONTROL\n");
            
            return 1;
        }
        case NV_ESC_RM_ACCESS_REGISTRY: {
            LOG("NV_ESC_RM_ACCESS_REGISTRY\n");
            
            return 1;
        }
        case NV_ESC_RM_MAP_MEMORY: {
            LOG("NV_ESC_RM_MAP_MEMORY\n");
            nv_ioctl_nvos33_parameters_with_fd *params = (nv_ioctl_nvos33_parameters_with_fd *)arg;
            LOG_VERBOSE("hMemory: 0x%08x, offset: 0x%016lx, length: 0x%016lx, flags: 0x%08x, address: %p, status: 0x%08x\n", params->params.hMemory, params->params.offset, params->params.length, params->params.flags, params->params.pLinearAddress, params->params.status);
            return 1;
        }
        case NV_ESC_RM_UNMAP_MEMORY: {
            LOG("NV_ESC_RM_UNMAP_MEMORY\n");
            NVOS34_PARAMETERS *params = (NVOS34_PARAMETERS *)arg;
            LOG_VERBOSE("hMemory: 0x%08x, address: %p, flags: 0x%08x, status: 0x%08x\n", params->hMemory, params->pLinearAddress, params->flags, params->status);
            return 1;
        }
        case NV_ESC_RM_GET_EVENT_DATA: {
            LOG("NV_ESC_RM_GET_EVENT_DATA\n");
            
            return 1;
        }
        case NV_ESC_RM_ALLOC_CONTEXT_DMA2: {
            LOG("NV_ESC_RM_ALLOC_CONTEXT_DMA2\n");
            
            return 1;
        }
        case NV_ESC_RM_ADD_VBLANK_CALLBACK: {
            LOG("NV_ESC_RM_ADD_VBLANK_CALLBACK\n");
            
            return 1;
        }
        case NV_ESC_RM_MAP_MEMORY_DMA: {
            LOG("NV_ESC_RM_MAP_MEMORY_DMA\n");
            NVOS46_PARAMETERS *params = (NVOS46_PARAMETERS *)arg;
            LOG_VERBOSE("hMemory: 0x%08x, offset: 0x%016lx, length: 0x%016lx, flags: 0x%08x, dmaOffset: 0x%016lx, status: 0x%08x\n", params->hMemory, params->offset, params->length, params->flags, params->dmaOffset, params->status);
            return 1;
        }
        case NV_ESC_RM_UNMAP_MEMORY_DMA: {
            LOG("NV_ESC_RM_UNMAP_MEMORY_DMA\n");
            NVOS47_PARAMETERS *params = (NVOS47_PARAMETERS *)arg;
            LOG_VERBOSE("hMemory: 0x%08x, flags: 0x%08x, dmaOffset: 0x%016lx, size: 0x%016lx, status: 0x%08x\n", params->hMemory, params->flags, params->dmaOffset, params->size, params->status);
            return 1;
        }
        case NV_ESC_RM_BIND_CONTEXT_DMA: {
            LOG("NV_ESC_RM_BIND_CONTEXT_DMA\n");
            
            return 1;
        }
        case NV_ESC_RM_EXPORT_OBJECT_TO_FD: {
            LOG("NV_ESC_RM_EXPORT_OBJECT_TO_FD\n");
            
            return 1;
        }
        case NV_ESC_RM_IMPORT_OBJECT_FROM_FD: {
            LOG("NV_ESC_RM_IMPORT_OBJECT_FROM_FD\n");
            
            return 1;
        }
        case NV_ESC_RM_UPDATE_DEVICE_MAPPING_INFO: {
            LOG("NV_ESC_RM_UPDATE_DEVICE_MAPPING_INFO\n");
            
            return 1;
        }
        case NV_ESC_RM_LOCKLESS_DIAGNOSTIC: {
            LOG("NV_ESC_RM_LOCKLESS_DIAGNOSTIC\n");
            
            return 1;
        }
    }
    return 0;
}